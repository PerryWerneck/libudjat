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
 #include <udjat/tools/variant.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/intl.h>
 #include <private/service.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>

 #ifdef _WIN32
	#include <private/win32/mainloop.h>
 #else
	#include <private/linux/mainloop.h>
 #endif // _WIN32

 #include <udjat/tools/service.h>

 using namespace std;

 namespace Udjat {

	Service::Service(const char *name, const char *description) : service_name{name}, service_description{description} {
		if(!(service_name && *service_name)) {
			throw system_error(EINVAL,system_category(),"Cant create unnamed service");
		}
		Service::Controller::getInstance().push_back(this);
	}

	Service::~Service() {
		Service::Controller::getInstance().remove(this);
	}

	bool Service::for_each(const std::function<bool(Service &service)> &method) {
		return Service::Controller::getInstance().for_each(method);
	}

	void Service::start() {
		state.active = true;
	}

	void Service::stop() {
		state.active = false;
	}

	Value & Service::get_properties(Value &properties) const {
		properties["name"] = service_name;
		properties["description"] = service_description;
		properties["active"] = state.active;
		return properties;
	}

	const Service * Service::find(const char *name) noexcept {
		return Controller::getInstance().find(name);
	}

	bool Service::get_property(const char *key, Value &value) const {

		if(strcasecmp(key,"name") == 0) {
			value = service_name;
		} else if(strcasecmp(key,"description") == 0) {
			value = service_description;
		} else if(strcasecmp(key,"active") == 0) {
			value = state.active;
		} else {
			return false;
		}

		return true;
	}

	bool Service::Controller::process(Request &request, Response &response) const noexcept {

		if(request != HTTP::Get) {
			response = HTTP::MethodNotAllowed;
			return true;
		}

		if(!request.allow(Authentication::Admin)) {
			response = HTTP::Forbidden;
			return true;
		}

		if(request.root()) {
			response = HTTP::BadRequest;
			return true;	
		}

		for(const auto service : *this) {
			if(request.pop(service->name()) && request.root()) {
				service->get_properties(response);
				return true;
			}
		}

		response = HTTP::NotFound;
		return true;
	}

	bool Service::Controller::schema(const HTTP::Method method, const char *, Schema::Output &schema) const noexcept {

		if(method != HTTP::Get) {
			return false;
		}

		schema.add(
			schema.Enumerable,
			Schema::Item{ "name", 			Schema::ObjectPath,	_("The Service name")			},
			Schema::Item{ "description", 	Schema::String,		_("The Service description")	},
			Schema::Item{ "active", 		Schema::Boolean,	_("Service state")				}			
		);

		return true;
	}

	bool Service::Controller::for_each(const std::function<bool(const Udjat::Variant &value)> &func) const noexcept {

		debug("Enumerating ",objects.size()," services");
		for(const auto service : objects) {
			Variant value;
			value["name"] = service->name();
			value["description"] = service->description();
			value["active"] = service->active();
			if(func(value)) {
				return true;
			}
		}

		return false;

	}

 }
