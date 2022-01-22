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
 #include <cstring>

 namespace Udjat {

	mutex Protocol::Controller::guard;

	Protocol::Controller & Protocol::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Protocol::Controller instance;
		return instance;
	}

	Protocol::Controller::Controller() {
	}

	Protocol::Controller::~Controller() {
	}

	void Protocol::Controller::insert(Protocol *protocol) {
		lock_guard<mutex> lock(guard);
#ifdef DEBUG
		cout << "Inserting protocol " << protocol->name << endl;
#endif // DEBUG
		protocols.push_back(protocol);
	}

	void Protocol::Controller::remove(Protocol *protocol) {
		lock_guard<mutex> lock(guard);
		protocols.remove(protocol);
	}

	const Protocol & Protocol::Controller::find(const char *name) {

		{
			/// @brief Singleton for file protocol.
			static File file;
		}

#ifdef DEBUG
		cout << "Searching for protocol '" << name << "'" << endl;
#endif // DEBUG

		lock_guard<mutex> lock(guard);
		for(auto protocol : protocols) {
			if(!strcasecmp(protocol->name,name)) {
				return *protocol;
			}
		}

		throw system_error(ENOENT,system_category(),string{"Cant find protocol '"} + name + "'");
	}

	void Protocol::Controller::getInfo(Udjat::Response &response) noexcept {

		response.reset(Value::Array);

		for(auto protocol : protocols) {

			Value &object = response.append(Value::Object);
			object["id"] = protocol->name;
//			protocol->info->get(object);

		}

	}

 }
