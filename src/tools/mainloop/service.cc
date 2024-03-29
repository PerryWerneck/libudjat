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
 #include <udjat/defs.h>
 #include <cstring>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/service.h>
 #include <private/service.h>

 #ifdef _WIN32
	#include <private/win32/mainloop.h>
 #else
	#include <private/linux/mainloop.h>
 #endif // _WIN32

 #include <udjat/tools/service.h>

 using namespace std;

 namespace Udjat {

	Service::Service(const char *name, const ModuleInfo &i) : module(i), service_name(name) {
		if(!service_name) {
			service_name = strrchr(module.name,'-');
			if(service_name) {
				service_name++;
			} else {
				service_name = module.name;
			}
		}

		Service::Controller::getInstance().push_back(this);

	}

	Service::Service(const ModuleInfo &module) : Service(nullptr,module) {
	}

	Service::~Service() {
		Service::Controller::getInstance().remove(this);
	}

	bool Service::for_each(const std::function<bool(const Service &service)> &method) {
		return Service::Controller::getInstance().for_each(method);
	}

	void Service::start() {
		state.active = true;
	}

	void Service::stop() {
		state.active = false;
	}

	std::ostream & Service::info() const {
		cout << name() << "\t";
		return cout;
	}

	std::ostream & Service::warning() const {
		clog << name() << "\t";
		return clog;
	}

	std::ostream & Service::error() const {
		cerr << name() << "\t";
		return cerr;
	}

	Value & Service::getProperties(Value &properties) const {
		properties["name"] = service_name;
		properties["active"] = state.active;
		return module.getProperties(properties);
	}

	const Service * Service::find(const char *name) noexcept {
		return Controller::getInstance().find(name);
	}

 }
