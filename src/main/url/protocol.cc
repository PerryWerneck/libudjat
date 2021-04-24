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
 #include <json/reader.h>

 namespace Udjat {

	URL::Protocol::Protocol(const Quark &protocol, const Quark &pn) : name(protocol), portname(pn) {

		static const ModuleInfo info;
		this->info = &info;

#ifdef DEBUG
		cout << "Protocol '" << name << "' created using port '" << portname << "'" << endl;
#endif // DEBUG
	}

	URL::Protocol::~Protocol() {
#ifdef DEBUG
		cout << "Protocol '" << name << "' destroyed" << endl;
#endif // DEBUG
	}

	std::shared_ptr<URL::Response> URL::Protocol::call(const URL &url, const URL::Method method, const char *mimetype, const char *payload) {
		throw runtime_error(string{"No back-end protocol for '"} + url.to_string() + "'");
	}

	int URL::Protocol::connect(const URL &url, time_t timeout) {
		throw runtime_error(string{"No back-end protocol for connect('"} + url.to_string() + "')");
	}


	Udjat::Response URL::Protocol::call(const URL &url, const Method method, const Request &payload) {

		Udjat::Response response;

		auto rsp = call(url,method,"application/json; charset=utf-8",payload.c_str());
		if(rsp->getStatusCode() != 200) {
			throw runtime_error(rsp->getStatusMessage());
		}

		{
			const char *text = rsp->c_str();
			Json::CharReaderBuilder builder;
			JSONCPP_STRING err;
			const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
			if (!reader->parse(text, text+strlen(text), &response, &err)) {
				throw runtime_error(err);
			}
		}

		/*
		// https://stackoverflow.com/questions/31121378/json-cpp-how-to-initialize-from-string-and-get-string-value
		Json::Reader reader;
		if(!reader.parse(rsp->c_str(), response)) {
			throw runtime_error(reader.getFormattedErrorMessages());
		}
		*/

		return response;
	}

 }
