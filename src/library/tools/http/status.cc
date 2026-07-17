/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/http/status.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/http/statuscodes.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>
 #include <sstream>
 #include <libintl.h>

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	// https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
	static const struct {
		HTTP::StatusCode	http;
		int 				system;
	} syscodes[] = {
		{ HTTP::Ok,					0			},
		{ HTTP::UnAuthenticated,	EPERM 		},
		{ HTTP::Forbidden,			EPERM 		},
		{ HTTP::NotFound,			ENOENT		},
		{ HTTP::MethodNotAllowed,	EINVAL		},
		{ HTTP::ProxyAuthRequired,	EPERM	 	},
#ifdef ETIMEDOUT
		{ HTTP::RequestTimeout,		ETIMEDOUT 	},
#endif // ETIMEDOUT
		{ HTTP::NotImplemented,		ENOTSUP		},
		{ HTTP::Unavailable,		EBUSY	 	},
		{ HTTP::NotFound,			ENODATA		},
		{ HTTP::Unprocessable,		ENOENT		},
	};

	HTTP::Status::Status(const std::exception &e) {
		assign(e);
	}

	HTTP::Status & HTTP::Status::clear() noexcept {
		code = HTTP::Ok;

		title.clear();
		message.clear();
		body.clear();
		domain.clear();
		url.clear();
		category.clear();
		return *this;

	}

	int HTTP::Status::syscode(const StatusCode code) noexcept {

		for(const auto &syscode : syscodes) {
			if(syscode.http == code) {
				return syscode.system;
			}
		}

		return -1;
	}

	std::string HTTP::Status::to_string(const MimeType &mimetype) const {
		stringstream out;
		serialize(mimetype,out);
		return out.str();
	}

	void HTTP::Status::serialize(const MimeType &mimetype, std::ostream &out) const noexcept {

		Value response{Value::Object};
		response["code"] = (int) code;
		response["title"] = title;
		response["message"] = message;
		response["body"] = body;
		response["domain"] = domain;
		response["url"] = url;
		response["category"] = category;	
		
		string value{code == HTTP::Ok ? "success" : "failed"};

		switch(mimetype) {
		case Udjat::Value::Undefined:
			Logger::String{"Unable to serialize undefined value"}.error("http");
			break;

		case Udjat::MimeType::xml:
			out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><response><status type='String'>";
			out << value << "</status>";
			out << "<data>";
			response.to_xml(out);
			out << "</data></response>";
			break;

		case Udjat::MimeType::json:

			// Reference: https://github.com/omniti-labs/jsend
			out << "{\"status\":\"" << value << "\",\"data\":";
			response.to_json(out);
			out << "}";
			break;

		case Udjat::MimeType::yaml:
			out << "status: \"" << value << "\"" << endl << "data:";
			response.to_yaml(out,4);
			break;

		case Udjat::MimeType::html:
			if(code == HTTP::Ok) {
				// Show values
				response.to_html(out);
			} else {
				out << "<section id='error-box'><h1 id='error-title'>" << (title.empty() ? _("Failed.") : title.c_str()) << "</h1>";
				if(!message.empty()) {
					out << "<p id='error-message'>" << message << "</p>";
				}
				out << "<p id='error-code'>" << "Error " << ((int) code) << "</p>";
				if(!body.empty()) {
					out << "<small id='error-body'>" << body << "</small>";
				}
				out << "<div id='error-extra'>";
				response.erase("title");
				response.erase("message");
				response.erase("body");
				response.erase("syscode");
				response.to_html(out);
				out << "</div>";
				out << "</section>";
			}
			break;

		case MimeType::sh:
			out << "status=\"" << value << "\"" << endl;
			response.to_sh(out);
			break;

		default:
			response["status"] = std::to_string(value);
			response.serialize(out,mimetype);

		}

	}

	HTTP::Status & HTTP::Status::assign(const std::exception &e) noexcept {

		clear();

		code = HTTP::SystemError;
		title = _("Unable to Complete Request");
		message = _("We're sorry, but we encountered an error while processing your request.");
		body = e.what();
		
		{
			const Udjat::Exception *except = dynamic_cast<const Udjat::Exception *>(&e);
			if(except) {
				assign(except->syscode());
				title = except->title();
				body = except->body();
				domain = except->domain();
				url = except->url();
				return *this;
			}
		}

		{
			const std::system_error *except = dynamic_cast<const std::system_error *>(&e);
			if(except) {
				assign(except->code().value());
				body = Logger::Message{
					_("The system error was '{}'"),
					except->code().message()
				};
				category = except->code().category().name();
				return *this;
			}
		}

		return *this;
	}

	HTTP::Status & HTTP::Status::assign(HTTP::StatusCode code) noexcept {

		clear();
		this->code = code;

		if(code >= (HTTP::StatusCode) 500 && code <= (HTTP::StatusCode) 599) {
			message = _("We're sorry, but we encountered an error while processing your request.");
			return *this;
		}

		message = std::to_string(code);
		return *this;
	}

	HTTP::Status & HTTP::Status::assign(int syscode) {

		clear();

		title = _("System error");
		message = _("We're sorry, but we encountered an error while processing your request.");

		code = HTTP::SystemError;

		body = Logger::Message{
			_("The system error was '{}'"),
			strerror(syscode)
		};

		for(const auto &item : syscodes) {
			if(item.system == syscode) {
				code = item.http;
				break;
			}
		}

		return *this;
	}

	HTTP::Status & HTTP::Status::failed(int syscode) noexcept {
		clear();
		assign(syscode);
		return *this;
	}

	HTTP::Status & HTTP::Status::failed(const char *message, const char *details) noexcept {
		return failed("",message,details);
	}

	HTTP::Status & HTTP::Status::failed(const char *title,  const char *message, const char *body) noexcept {

		code = HTTP::SystemError;

		if(title && *title) {
			title = title;
		} else {
			title = _("Unable to Complete Request");
		}

		bool has_message = (message && *message);
		bool has_body = (body && *body);

		if(has_message && has_body) {
			this->message = message;
			this->body = body;
		} else if(has_message) {
			this->message = _("We're sorry, but we encountered an error while processing your request.");
			this->body = message;
		} else if(has_body) {
			this->message = _("We're sorry, but we encountered an error while processing your request.");
			this->body = body;
		} else {
			this->message = _("We're sorry, but we encountered an error while processing your request.");
			this->body = _("Unexpected error processing request");
		}

		return *this;

	}

}

 namespace std {

	UDJAT_API const string to_string(const Udjat::HTTP::StatusCode code) {

		static const struct {
			HTTP::StatusCode code;
			const char *text;
		} messages[] = {
			{
				HTTP::NotFound,
				N_("Not available")
			},
			{
				HTTP::Forbidden,
				N_("You dont have access to this resource")
			},
			{	
				HTTP::RequestTimeout,
				N_("Request timeout")
			},
			{ 
				HTTP::SystemError,
				N_("Internal Server Error")
			},
			{
				HTTP::NotImplemented,
				N_("The request method is not supported by the server and cannot be handled.")
			}
		};

		for(const auto &msg : messages) {
			if(msg.code == code) {
				return dgettext(GETTEXT_PACKAGE,msg.text);
			}
		}

		return Udjat::Logger::Message{_("HTTP Status {}"),(int) code};
	}

 }
