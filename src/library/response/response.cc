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

	Response::~Response() {
	}

	time_t Response::expires(const time_t tm) noexcept {
		if(tm && (!timestamp.expires || timestamp.expires > tm)) {
			timestamp.expires = tm;
		}
		return timestamp.expires;
	}

	void Response::state(const char *,const char *, const char *) {		
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

	void Response::header(const char *, const char *) noexcept {
	}

	HTTP::Status & Response::assign(const HTTP::StatusCode code) noexcept {
		debug("Request set to HTTP status ",code);
		clear(Value::Object);
		return status.assign(code);
	}

	HTTP::Status & Response::failed(int syscode) noexcept {
		debug("Request failed with syscode ",syscode);
		clear(Value::Object);
		return status.failed(syscode);
	}

	HTTP::Status & Response::failed(const char *message, const char *details) noexcept {
		return status.failed(message,details);
	}

	HTTP::Status & Response::failed(const char *title,  const char *message, const char *body) noexcept {
		clear(Value::Object);
		return status.failed(title,message,body);
	}

	HTTP::Status & Response::failed(const std::exception &e) noexcept {
		status.assign(e);
		return status;
	}

	void Response::serialize(std::ostream &stream) const noexcept {

		debug("Serializing response");

		if(status.code == HTTP::NoContent) {
			// No Content status, the response should be empty.
			return;
		}

		if(status.code != HTTP::Ok) {
			status.serialize(mimetype,stream);
			return;
		}

		string value{status.code == HTTP::Ok ? "success" : "failed"};

		switch(mimetype) {
		case Udjat::Value::Undefined:
			{
				HTTP::Status st{HTTP::SystemError};
				st.failed(_("Unable to serialize undefined value"));
				st.serialize(mimetype,stream);
			}
			break;

		case Udjat::MimeType::xml:
			stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><response><status type='String'>";
			stream << value << "</status>";
			stream << "<code>" << status.code << "</code>";
			if(!status.message.empty()) {
				stream << "<message>" << status.message << "</message>";
			}
			stream << "<data>";
			to_xml(stream);
			stream << "</data></response>";
			break;

		case Udjat::MimeType::json:

			// Reference: https://github.com/omniti-labs/jsend
			stream << "{\"status\":\"" << value << "\",\"data\":";
			to_json(stream);
			stream << "}";
			break;

		case Udjat::MimeType::yaml:
			stream << "status: \"" << value << "\"" << endl << "data:";
			to_yaml(stream,4);
			break;

		case Udjat::MimeType::html:
			if(status.code == HTTP::Ok) {
				// Show values
				to_html(stream);
			} else {
				stream << "<section id='error-box'><h1 id='error-title'>" << (status.title.empty() ? _("Operation failed") : status.title.c_str()) << "</h1>";
				if(!status.message.empty()) {
					stream << "<p id='error-message'>" << status.message << "</p>";
				} 
				stream << "<p id='error-code'>" << "Error " << status.code << "</p>";
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
			stream << "status=\"" << value << "\"" << endl;
			to_sh(stream);
			break;

		default:
			Value::serialize(stream,mimetype);
		}

	}

 }
