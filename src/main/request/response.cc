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

	void Abstract::Response::setModificationTimestamp(const time_t time) {

		if(modification)
			modification = min((time_t) modification,time);
		else
			modification = time;

	}

	void Response::Value::serialize(std::ostream &stream, const MimeType mimetype) const {

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
			Value::serialize(stream,mimetype);
		}

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

