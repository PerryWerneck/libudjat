/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/url.h>
 #include <udjat/tools/container.h>

 using namespace std;

 namespace Udjat {

	static Container<URL::Handler> & Controller() {
		static Container<URL::Handler> instance;
		return instance;
	}

	const URL::Handler & URL::handler(const char *name) {
		for(const URL::Handler *handler : Controller()) {
			if(*handler == name) {
				return *handler;
			}
		}
		throw runtime_error(String{"Unable to handle '",name,"' urls"});
	}

	URL::Handler::Handler(const char *name) : handler_name{name} {
		Controller().push_back(this);
	}

	URL::Handler::~Handler() {
		Controller().remove(this);
	}

	String URL::Handler::get(const URL &url, const MimeType mimetype) const {
		return get(&url, [](uint64_t current, uint64_t total){return false;},mimetype);
	}

	bool URL::Handler::get(const URL &url, const char *filename, const MimeType mimetype) const {
		return get(&url, filename, [](uint64_t current, uint64_t total){return false;},mimetype);
	}

 }

