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
 #include <udjat/tools/response.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/value.h>
 #include <ctime>
 #include <stdexcept>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	Response::Status::Status(const std::exception &e) {
		assign(e);
	}

	Response::Status & Response::Status::clear(const State st) noexcept {
		value = st;
		syscode = st == Success ? 0 : -1;
		not_modified = false;
		title.clear();
		message.clear();
		body.clear();
		domain.clear();
		url.clear();
		category.clear();
		return *this;
	}

	std::string Response::Status::to_string(const MimeType &mimetype) const {
		stringstream out;
		serialize(mimetype,out);
		return out.str();
	}

	void Response::Status::serialize(const MimeType &mimetype, std::ostream &out) const {

		Value response{Value::Object};
		response["title"] = title;
		response["message"] = message;
		response["body"] = body;
		response["domain"] = domain;
		response["url"] = url;
		response["category"] = category;		

		switch(mimetype) {
		case Udjat::Value::Undefined:
			throw runtime_error("Unable to serialize undefined value");
			break;

		case Udjat::MimeType::xml:
			out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><response><status type='String'>";
			out << value << "</status>";
			out << "<code>" << syscode << "</code>";
			out << "<message>" << message << "</message>";
			out << "<data>";
			response.to_xml(out);
			out << "</data></response>";
			break;

		case Udjat::MimeType::json:

			// Reference: https://github.com/omniti-labs/jsend
			out << "{\"status\":\"" << value << "\",\"data\":";
			response["code"] = syscode;
			response.to_json(out);
			out << "}";
			break;

		case Udjat::MimeType::yaml:
			out << "status: \"" << value << "\"" << endl << "data:";
			response.to_yaml(out,4);
			break;

		case Udjat::MimeType::html:
			if(value == Success) {
				// Show values
				response.to_html(out);
			} else {
				out << "<section id='error-box'><h1 id='error-title'>" << (title.empty() ? _("We're sorry, but we encountered an error while processing your request.") : title.c_str()) << "</h1>";
				if(!message.empty()) {
					out << "<p id='error-message'>" << message << "</p>";
				} else if(syscode) {
					out << "<p id='error-code'>" << "Error " << syscode << "</p>";
				}
				if(!body.empty()) {
					out << "<small id='error-body'>" << body << "</small>";
				}
				out << "<div id='error-extra'>";
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
			response["code"] = syscode;
			response.serialize(out,mimetype);

		}

	}

	Response::Status & Response::Status::assign(const std::exception &e) noexcept {

		clear();

		value = Failure;
		syscode = -1;
		title = _("Unable to Complete Request");
		message = _("We're sorry, but we encountered an error while processing your request.");
		body = e.what();
		
		{
			const Udjat::Exception *except = dynamic_cast<const Udjat::Exception *>(&e);
			if(except) {

				syscode = except->syscode();
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

				syscode = except->code().value();
				title = _("System error");
				body = except->code().message();
				category = except->code().category().name();
				
				return *this;

			}
		}

		return *this;
	}

	Response::~Response() {
	}

	time_t Response::expires(const time_t tm) noexcept {
		if(tm && (!timestamp.expires || timestamp.expires > tm)) {
			timestamp.expires = tm;
		}
		return timestamp.expires;
	}

	time_t Response::last_modified(const time_t tm) noexcept {
		if(tm && (!timestamp.last_modified || timestamp.last_modified < tm)) {
			timestamp.last_modified = tm;
		}
		return timestamp.last_modified;
	}

	const char * Response::message() const noexcept {
		if(status.message.empty()) {
			return "Ok";
		}
		return status.message.c_str();
	}

	Response & Response::failed(int syscode) noexcept {
		status.value = State::Failure;
		clear(Value::Object);
		status.message = strerror(syscode);
		status.syscode = syscode;
		return *this;
	}

	Response & Response::failed(const char *message, const char *details) noexcept {
		return failed("",message,details);
	}

	Response & Response::failed(const char *title,  const char *message, const char *details) noexcept {

		status.value = State::Failure;
		clear(Value::Object);

		status.message = message;
		status.syscode = 0;

		if(title && *title) {
			status.title = title;
		} else {
			status.title.clear();
		}

		if(details && *details) {
			status.body = details;
		} else {
			status.body.clear();
		}

		return *this;

	}

	Response & Response::failed(const std::exception &e) noexcept {
		status.assign(e);
		return *this;
	}

	void Response::serialize(std::ostream &stream) const {

		debug("Serializing response");

		if(status.value != Success) {
			status.serialize(mimetype,stream);
			return;
		}

		// response["title"] = status.title;
		// response["message"] = status.message;
		// response["body"] = body;
		// response["domain"] = domain;
		// response["url"] = url;
		// response["category"] = category;		

		switch(mimetype) {
		case Udjat::Value::Undefined:
			throw runtime_error("Unable to serialize undefined value");
			break;

		case Udjat::MimeType::xml:
			stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><response><status type='String'>";
			stream << status.value << "</status>";
			if(status.syscode) {
				stream << "<code>" << status.syscode << "</code>";
			}
			if(!status.message.empty()) {
				stream << "<message>" << status.message << "</message>";
			}

			stream << "<data>";
			to_xml(stream);
			stream << "</data></response>";
			break;

		case Udjat::MimeType::json:

			// Reference: https://github.com/omniti-labs/jsend
			stream << "{\"status\":\"" << status.value << "\",\"data\":";
			to_json(stream);
			stream << "}";
			break;

		case Udjat::MimeType::yaml:
			stream << "status: \"" << status.value << "\"" << endl << "data:";
			to_yaml(stream,4);
			break;

		case Udjat::MimeType::html:
			if(status.value == Success) {
				// Show values
				to_html(stream);
			} else {
				stream << "<section id='error-box'><h1 id='error-title'>" << (status.title.empty() ? _("Operation failed") : status.title.c_str()) << "</h1>";
				if(!status.message.empty()) {
					stream << "<p id='error-message'>" << status.message << "</p>";
				} else if(status.syscode) {
					stream << "<p id='error-code'>" << "Error " << status.syscode << "</p>";
				}
				if(!status.body.empty()) {
					stream << "<small id='error-details'>" << status.body << "</small>";
				}
				if(!empty()) {
					stream << "<div id='error-extra'>";
					to_html(stream);
					stream << "</div>";
				}
				stream << "</section>";
			}
			break;

		case MimeType::sh:
			stream << "status=\"" << status.value << "\"" << endl;
			to_sh(stream);
			break;

		default:
			Value::serialize(stream,mimetype);
		}

	}

 }

namespace std {

	UDJAT_API const char * to_string(const Udjat::Response::State state) {

		static const char * names[] = {
			"success",
			"error",
			"failure"
		};

		if( ((size_t) state) >= N_ELEMENTS(names)) {
			return "failure";
		}

		return names[state];

	}

 }
