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
 #include <private/protocol.h>
 #include <udjat/tools/protocol.h>
 #include <cstring>
 #include <udjat/module/info.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/logger.h>

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

	void Protocol::Controller::insert(Protocol::Worker *worker) {
		lock_guard<mutex> lock(guard);
		workers.push_back(worker);
	}

	void Protocol::Controller::remove(Protocol::Worker *worker) {
		lock_guard<mutex> lock(guard);
		workers.remove(worker);
	}

	void Protocol::Controller::insert(Protocol *protocol) {
		lock_guard<mutex> lock(guard);
		Logger::String {
			"Register '",protocol->name,"' (",protocol->module.description,")"
		}.trace("protocols");
		protocols.push_back(protocol);
	}

	void Protocol::Controller::remove(Protocol *protocol) {

		lock_guard<mutex> lock(guard);

		Logger::String {
			"Unregister '",protocol->name,"' (",protocol->module.description,")"
		}.trace("protocols");

		if(def == protocol) {
			def = nullptr;
		}

		protocols.remove(protocol);
	}

	/*
	const Protocol * Protocol::Controller::find(const char *name, bool allow_default, bool autoload) {

		debug("Searching for protocol '",name,"'");

		{
			lock_guard<mutex> lock(guard);
			for(auto protocol : protocols) {
				if(*protocol == name) {
					return protocol;
				}
			}
		}

		if(autoload) {
			Logger::String{"Autoloading module '",name,"'"}.trace("protocols");
			Module::load(name);
			return find(name, allow_default, false);
		}

		if(allow_default) {
			return def;
		}

		return nullptr;

	}
	*/

	const Protocol * Protocol::Controller::verify(const void *protocol) {

		lock_guard<mutex> lock(guard);
		for(auto prot : protocols) {
			if(prot == (Protocol *) protocol) {
				return prot;
			}
		}

		return nullptr;
	}

 }
