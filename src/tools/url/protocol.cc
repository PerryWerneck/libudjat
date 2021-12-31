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

	URL::Protocol::~Protocol() {
		cout << name << "\tProtocol unregistered" << endl;
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	std::shared_ptr<URL::Response> URL::Protocol::call(const URL &url, const URL::Method method, const char *mimetype, const char *payload) {
		throw runtime_error(string{"No back-end protocol for '"} + url.to_string() + "'");
	}
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	int URL::Protocol::connect(const URL &url, time_t timeout) {
		throw runtime_error(string{"No back-end protocol for connect('"} + url.to_string() + "')");
	}
	#pragma GCC diagnostic pop

	std::string URL::Protocol::call(const URL &url, const Method method, const Request &payload) {

		auto rsp = call(url,method,"application/json; charset=utf-8",payload.getPath());
		if(rsp->getStatusCode() != 200) {
			throw runtime_error(rsp->getStatusMessage());
		}

		return rsp->c_str();
	}

 }
