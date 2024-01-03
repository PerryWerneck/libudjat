/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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
 #include <algorithm>
 #include <private/request.h>
 #include <udjat/tools/response.h>
 #include <ctime>

 using namespace std;

 namespace Udjat {

	void Abstract::Response::setExpirationTimestamp(const time_t time) {

		if(expiration) {
			expiration = min((time_t) expiration,time);
		} else {
			expiration = time;
		}

	}

	time_t Abstract::Response::last_modified(const time_t tm) {

		if(tm && tm <= time(0)) {
			if(!modification || modification > tm)
				modification = tm;
		}

		return (time_t) modification;

	}

	std::string Response::Value::to_string() const {
		return Udjat::Value::to_string(mimetype);
	}

	void Response::Value::serialize(std::ostream &stream) const {
		serialize(stream,this->mimetype);
	}

	void Response::Value::serialize(std::ostream &stream, const MimeType mimetype) const {

		switch(mimetype) {
		case Udjat::MimeType::xml:

			// Format as XML

			stream	<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?><response status='"
					<< (this->success ? "success" : "failed") << "'";

			if(modification) {
				stream << " timestamp='" << modification.to_string() << "'";
			}

			if(expiration) {
				stream << " expires='" << expiration.to_string() << "'";
			}

			stream << ">";
			to_xml(stream);
			stream << "</response>";

			break;

		case Udjat::MimeType::json:

			// Format as json
			// https://stackoverflow.com/questions/12806386/is-there-any-standard-for-json-api-response-format

			// https://github.com/omniti-labs/jsend
			stream << "{\"status\":\"";
			stream << this->success ? "success" : "failed";
 			stream << "\",\"data\":";
			Udjat::Value::serialize(stream,mimetype);
			stream << "}";

			break;

		default:
			Udjat::Value::serialize(stream,mimetype);
		}

		/*
		if(mimetype == Udjat::MimeType::xml) {
			// Format as XML
			stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><response";

			if(modification) {
				stream << " timestamp='" << modification.to_string() << "'";
			}

			if(expiration) {
				stream << " expires='" << expiration.to_string() << "'";
			}

			stream << ">";
			to_xml(stream);
			stream << "</response>";
		} else {
			Udjat::Value::serialize(stream,mimetype);
		}
		*/

	}

	Response::Value::operator Value::Type() const noexcept {
		return Value::Object;
	}

	bool Response::Value::isNull() const {
		return false;
	}

	Udjat::Value & Response::Value::reset(const Udjat::Value::Type) {
		throw system_error(EPERM,system_category(),"Response types are fixed");
	}

	Udjat::Value & Response::Value::set(const Udjat::Value &) {
		throw system_error(EPERM,system_category(),"Response types are fixed");
	}


 }

