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
 #include <udjat/tools/variant.h>
 #include <udjat/tools/http/status.h>
 #include <ctime>
 #include <stdexcept>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	Response::~Response() {
	}

	time_t Response::expires(const time_t tm) noexcept {
		if(tm && (!http_status.expires || http_status.expires > tm)) {
			http_status.expires = tm;
		}
		return http_status.expires;
	}

	time_t Response::last_modified(const time_t tm) noexcept {
		if(tm && (!http_status.last_modified || http_status.last_modified < tm)) {
			http_status.last_modified = tm;
		}
		return http_status.last_modified;
	}

	const char * Response::message() const noexcept {
		if(http_status.message.empty()) {
			return "Ok";
		}
		return http_status.message.c_str();
	}

	void Response::header(const char *, const char *) noexcept {
	}

	Response & Response::assign(const HTTP::StatusCode code, const char *body) noexcept {
		debug("Request set to HTTP status ",code);

		// Reminder: DO NOT CLEAR the contents, the dbus engine use it to keep states.

		http_status.assign(code,body);
		return *this;
	}

	Response & Response::assign(const std::exception &e) noexcept {
		http_status.assign(e);
		return *this;
	}

	void Response::serialize(std::ostream &stream) const {

		debug(
			"Serializing response with mimetype ", 
			std::to_string(http_status.mimetype)
		);

		if(http_status.code == HTTP::NoContent || http_status.code == HTTP::NotModified) {
			// No Content or not-modified status, the response should be empty.
			return;
		}


		// If failed send only the status.
		if(http_status.failed()) {
			http_status.serialize(stream);
			return;
		}

		static const char *value = "success";

		// Not failed, serialize values.
		switch(http_status.mimetype) {
		case Udjat::Variant::Undefined:
			{
				debug("Undefined value, error");
				HTTP::Status st{HTTP::SystemError,MimeType::html};
				st.failed(_("Unable to serialize response with undefined mimetype"));
				st.serialize(stream);
			}
			break;

		case Udjat::MimeType::xml:
			stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><response><status type='String'>";
			stream << value << "</status>";
			stream << "<code>" << http_status.code << "</code>";
			if(!http_status.message.empty()) {
				stream << "<message>" << http_status.message << "</message>";
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
			if(http_status.code == HTTP::Ok) {
				// Show values
				to_html(stream);
			} else {
				stream << "<section id='error-box'><h1 id='error-title'>" << (http_status.title.empty() ? _("Operation failed") : http_status.title.c_str()) << "</h1>";
				if(!http_status.message.empty()) {
					stream << "<p id='error-message'>" << http_status.message << "</p>";
				} 
				stream << "<p id='error-code'>" << "Error " << http_status.code << "</p>";
				if(!http_status.body.empty()) {
					stream << "<small id='error-details'>" << http_status.body << "</small>";
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
			Variant::serialize(stream,http_status.mimetype);
		}

	}

 }
