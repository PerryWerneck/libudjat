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

 #include "private.h"

 namespace Udjat {

	URL::Response::~Response() {
	}

	bool URL::Response::isValid() const noexcept {
		return this->response.payload && this->response.payload[0];
	}

	const char * URL::Response::c_str() const {
		if(response.payload)
			return response.payload;
		throw system_error(ENODATA,system_category(),(status.text.empty() ? "Empty response" : status.text.c_str()));
	}

	void URL::Response::call(std::function<void()> method) noexcept {

		try {

			method();

		} catch(const HTTP::Exception &e) {

			status.text = e.what();
			status.code = e.codes().http;

		} catch(const std::exception &e) {

			status.code = 500;
			status.text = e.what();

		} catch(...) {

			status.code = 500;
			status.text = "Unexpected error";

		}

	}

	/*
	Json::Value URL::Response::as_json() const {

		cout << "ENTREI - convert" << endl;

		Json::Value value;
		Json::CharReaderBuilder builder;
		JSONCPP_STRING err;

		const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

		cout << "Payload:" << endl << response.payload << endl;

		if(response.payload) {

			if (!reader->parse(response.payload, response.payload+response.length, &value, &err)) {
				throw runtime_error(err);
			}

		} else {

			// TODO: Create json.
			Json::Value error(Json::objectValue);
			error["code"] = status.code;
			error["message"] = status.text;
			value["error"] = error;

		}

		return value;

	}
	*/

 }
