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

 static const char * method_names[] = {
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"CONNECT",
	"OPTIONS",
	"TRACE",
	"PATCH",
 };

 namespace Udjat {

	HTTP::Method HTTP::MethodFactory(const char *name) {
		for(size_t ix = 0; ix < (sizeof(method_names)/sizeof(method_names[0])); ix++) {
			if(!strcasecmp(name,method_names[ix])) {
				return (Method) ix;
			}
		}
		throw system_error(EINVAL,system_category(),string{"The method '"} + name + "' is invalid");
	}

	Protocol::Protocol(const char *n, const ModuleInfo *i) : name(n), info(i) {
		Controller::getInstance().insert(this);
	}

	Protocol::~Protocol() {
		Controller::getInstance().remove(this);
	}

	const Protocol & Protocol::find(const URL &url) {
		return find(url.scheme().c_str());
	}

	const Protocol & Protocol::find(const char *name) {
		return Controller::getInstance().find(name);
	}

	void Protocol::getInfo(Udjat::Response &response) noexcept {
		Controller::getInstance().getInfo(response);
	}

	std::string Protocol::call(const char *u, const HTTP::Method method, const char *payload) {
		URL url(u);
		return find(url).call(u,method,payload);
	}

	std::string Protocol::call(const URL &url, const HTTP::Method UDJAT_UNUSED(method), const char UDJAT_UNUSED(*payload)) const {
		throw runtime_error(string {"Invalid protocol type for "} + url.c_str());
	}

	std::string Protocol::call(const URL &url, const char *method, const char *payload) const {
		return call(url,HTTP::MethodFactory(method), payload);
	}

 }

 namespace std {

	const char * to_string(const Udjat::HTTP::Method method) {
		if((size_t) method > (sizeof(method_names)/sizeof(method_names[0]))) {
			throw system_error(EINVAL,system_category(),"Invalid method id");
		}
		return method_names[method];
	}

 }

