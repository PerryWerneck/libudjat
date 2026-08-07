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
 #include <udjat/tools/response.h>
 #include <udjat/tools/intl.h>
 #include <ctime>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	Response::~Response() {
	}

	void Response::clear() noexcept {
		Variant::clear(Variant::Object);
		HTTP::Status::clear();
	}

	time_t Response::expires(const time_t tm) noexcept {
		if(tm && (!timestamp.expiration || timestamp.expiration > tm)) {
			timestamp.expiration = tm;
		}
		return timestamp.expiration;
	}

	time_t Response::last_modified(const time_t tm) noexcept {
		if(tm && (!timestamp.modification || timestamp.modification < tm)) {
			timestamp.modification = tm;
		}
		return timestamp.modification;
	}

	std::string Response::to_string() const noexcept {

		stringstream out;
		serialize(out);
		return out.str();

	}

	void Response::serialize(std::ostream &stream) const noexcept {

		debug("Serializing response with mimetype ", std::to_string(mimetype));

		if(code == HTTP::NoContent || code == HTTP::NotModified) {
			// No Content or not-modified status, the response should be empty.
			return;
		}

		// If failed send only the status.
		if(failed()) {
			HTTP::Status::serialize(stream);
			return;
		}

		// Not failed, format variant contents.
		switch(mimetype) {
		case Variant::Undefined:
			{
				HTTP::Status st{HTTP::SystemError,mimetype};
				st.failed(_("Unable to serialize response with undefined mimetype"));
				st.serialize(stream);
			}
			break;

		case Udjat::MimeType::xml:
			stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><response><status type='String'>success</status>";
			stream << "<code>" << code << "</code>";
			if(!message.empty()) {
				stream << "<message type='String'>" << message << "</message>";
			}
			if(!detail.empty()) {
				stream << "<detail type='String'>" << detail << "</detail>";
			}
			stream << "<data>";	
			Variant::to_xml(stream);
			stream << "</data></response>";
			break;

		case Udjat::MimeType::json:
			// Reference: https://github.com/omniti-labs/jsend
			stream << "{\"status\":\"success\",\"data\":";
			Variant::to_json(stream);
			stream << "}";
			break;

		case Udjat::MimeType::yaml:
			stream << "status: \"success\"" << endl << "data:";
			Variant::to_yaml(stream,4);
			break;

		case Udjat::MimeType::html:
			Variant::to_html(stream);
			break;

		case MimeType::sh:
			stream << "status=\"success\"" << endl;
			Variant::to_sh(stream);
			break;

		default:
			Variant::serialize(stream,mimetype);
		}

	}

 }



//  #include <udjat/tools/exception.h>
//  #include <udjat/tools/intl.h>
//  #include <udjat/tools/variant.h>
//  #include <udjat/tools/http/status.h>
//  #include <ctime>
//  #include <stdexcept>
//  #include <sstream>

//  using namespace std;

//  namespace Udjat {

// 	Response::~Response() {
// 	}


// 	const char * Response::message() const noexcept {
// 		if(http_status.message.empty()) {
// 			return "Ok";
// 		}
// 		return http_status.message.c_str();
// 	}

// 	void Response::header(const char *, const char *) noexcept {
// 	}

// 	Response & Response::assign(const HTTP::StatusCode code, const char *body) noexcept {
// 		debug("Request set to HTTP status ",code);

// 		// Reminder: DO NOT CLEAR the contents, the dbus engine use it to keep states.

// 		http_status.assign(code,body);
// 		return *this;
// 	}

// 	Response & Response::assign(const std::exception &e) noexcept {
// 		http_status.assign(e);
// 		return *this;
// 	}


//  }
