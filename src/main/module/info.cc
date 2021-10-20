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
 #include <udjat/tools/value.h>

#ifndef _WIN32
	#include <dlfcn.h>
#endif // _WIN32

 #include <iostream>
 #include <cstdarg>

 using namespace std;

 namespace Udjat {

	Value & ModuleInfo::get(Value &value) const {

		value["module"] = name;
		value["description"] = description;
		value["version"] = version;
		value["bugreport"] = bugreport;
		value["url"] = url;

#ifndef _WIN32

		Dl_info info;
		if(dladdr(this, &info) != 0) {

			if(info.dli_fname && info.dli_fname[0]) {
				value["filename"] = info.dli_fname;
			} else {
				value["filename"] = "";
			}
		}

#endif // _WIN32

		return value;
	}

 }
