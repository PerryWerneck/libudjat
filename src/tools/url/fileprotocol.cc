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
 #include <udjat/tools/file.h>

 namespace Udjat {

	static const ModuleInfo moduleinfo {
		PACKAGE_NAME,									// The module name.
		"File protocol module",	 						// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
	};

	Protocol::Controller::File::File() : Udjat::Protocol((const char *) "file",&moduleinfo) {
	}

	Protocol::Controller::File::~File() {
	}

	std::string Protocol::Controller::File::call(const URL &url, const HTTP::Method method, const char *payload) const {
		if(method != HTTP::Get) {
			throw system_error(EINVAL,system_category(),"Invalid request method");
		}
		return string(Udjat::File::Text(url.ComponentsFactory().path.c_str()).c_str());
	}

 }

