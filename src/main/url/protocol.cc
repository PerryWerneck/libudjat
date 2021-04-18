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

	URL::Protocol::Protocol(const Quark &protocol, const char *pn) : name(protocol), portname(pn) {

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

	void URL::Protocol::get(const URL &url, time_t timeout, std::function<void(const char *block, size_t len)> reader) {
		throw runtime_error("No back-end protocol for 'get'");
	}

	int URL::Protocol::connect(const URL &url, time_t timeout) {
		throw runtime_error("No back-end protocol for 'connect'");
	}

	void URL::Protocol::get(const URL &url, time_t timeout, Response &response) {

		string text;

		this->get(url,timeout,[&text](const char *block, size_t len){
			text.append(block,len);
		});

		// https://stackoverflow.com/questions/31121378/json-cpp-how-to-initialize-from-string-and-get-string-value

		Json::Reader reader;
		if(!reader.parse(text.c_str(), response)) {
			throw runtime_error(reader.getFormattedErrorMessages());
		}

	}

 }
