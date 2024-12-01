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
 #include <ctime>
 #include <stdexcept>
 #include <sstream>

 using namespace std;

 namespace Udjat {

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

	std::string Response::to_string() const noexcept {
		try {
			std::ostringstream out;
			serialize(out);
			return out.str();
		} catch(const std::exception &e) {
			Logger::String{"Error serializing response: ",e.what()}.error();
		} catch(...) {
			Logger::String{"Unexpected error serializing response"}.error();
		}
		return "";
	}

	Response & Response::failed(int syscode) noexcept {
		status.value = State::Failure;
		clear(Value::Object);
		status.message = strerror(syscode);
		status.code = syscode;
		return *this;
	}

	Response & Response::failed(const char *message, const char *details) noexcept {
		return failed("",message,details);
	}

	Response & Response::failed(const char *title,  const char *message, const char *details) noexcept {

		status.value = State::Failure;
		clear(Value::Object);

		status.message = message;
		status.code = 0;

		if(title && *title) {
			status.title = title;
			(*this)["title"] = status.title;
		} else {
			status.title.clear();
		}

		if(details && *details) {
			(*this)["details"] = status.details;
			status.details = details;
		} else {
			status.details.clear();
		}

		return *this;

	}

	Response & Response::failed(const std::exception &e) noexcept {

		status.value = State::Failure;
		clear(Value::Object);

		status.message = e.what();
		status.title.clear();
		status.details.clear();
		status.code = 0;

		{
			const Udjat::Exception *except = dynamic_cast<const Udjat::Exception *>(&e);
			if(except) {

				status.title = except->title();
				status.details = except->body();
				status.code = except->syscode();

				(*this)["title"] = status.title;
				(*this)["details"] = status.details;
				(*this)["domain"] = except->domain();
				(*this)["url"] = except->url();

				return *this;
			}
		}

		{
			const std::system_error *except = dynamic_cast<const std::system_error *>(&e);
			if(except) {

				status.code = except->code().value();
				status.title = _("System error");
				status.details = except->code().message();

				(*this)["title"] = status.title;
				(*this)["details"] = status.details;
				(*this)["category"] = except->code().category().name();
				
				return *this;

			}
		}

		return *this;
	}

	void Response::serialize(std::ostream &stream) const {

		// https://github.com/omniti-labs/jsend

		debug("Serializing response");

		switch(mimetype) {
		case Udjat::Value::Undefined:
			throw runtime_error("Unable to serialize undefined value");
			break;

		case Udjat::MimeType::xml:
			stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><response><status type='String'>";
			stream << status.value << "</status>";
			if(status.code) {
				stream << "<code>" << status.code << "</code>";
			}
			if(!status.message.empty()) {
				stream << "<message>" << status.message << "</message>";
			}

			stream << "<data>";
			to_xml(stream);
			stream << "</data></response>";
			break;

		case Udjat::MimeType::json:
			stream << "{\"status\":\"" << status.value << "\",\"data\":";
			to_json(stream);
			stream << "}";
			break;

		case Udjat::MimeType::yaml:
			stream << "status: \"" << status.value << "\"" << endl << "data:" << endl;
			to_yaml(stream,4);
			break;

		case Udjat::MimeType::html:
			stream << "<!doctype html xmlns=\"http://www.w3.org/1999/xhtml\">" \
						"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>"
					<< (status.message.empty() ? "Response" : status.message)
					<< "</title></head><body>";
			to_html(stream);
			stream << "</body></html>";
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
