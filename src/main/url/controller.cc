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
 #include <udjat/tools/value.h>

 namespace Udjat {

	URL::Controller::Controller() {
		insert(make_shared<FileProtocol>());
	}

	URL::Controller::~Controller() {

	}

	URL::Controller & URL::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	void URL::Controller::insert(std::shared_ptr<Protocol> protocol) {
		protocols.push_back(protocol);
	}

	void URL::Controller::getInfo(Udjat::Response &response) noexcept {

		response.reset(Value::Array);

		for(auto protocol : this->protocols) {

			Value &object = response.append(Value::Object);

			object["id"] = protocol->c_str();
			object["portname"] = protocol->getDefaultPortName();
			protocol->getModuleInfo()->get(object);

		}

	}

	shared_ptr<URL::Protocol> URL::Controller::find(const char *name) {

		for(auto protocol : protocols) {
			if(!strcmp(name,protocol->c_str())) {
				return protocol;
			}
		}

		throw runtime_error(string{"No back-end for scheme '"} + name + "'");
	}


 }
