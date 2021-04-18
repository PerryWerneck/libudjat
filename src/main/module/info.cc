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
 #include <dlfcn.h>
 #include <iostream>
 #include <cstdarg>

 using namespace std;

 namespace Udjat {

	ModuleInfo::ModuleInfo() : name(""), description(""), version(""), bugreport(""), url(""), path(nullptr) {
	}

	ModuleInfo::ModuleInfo(const char *str, ...) : ModuleInfo() {

		const char **arg[] = {
			&this->name,
			&this->description,
			&this->version,
			&this->url,
			&this->bugreport
		};

		va_list args;
		va_start (args, str);

		for(size_t ix = 0; ix < (sizeof(arg)/sizeof(arg[0])) && str; ix++) {
			*arg[ix] = str;
			str = va_arg(args, const char *);
		}

	}


	Json::Value & ModuleInfo::get(Json::Value &value) const {

		value["name"] = name;
		value["description"] = description;
		value["version"] = version;
		value["bugreport"] = bugreport;
		value["url"] = url;

		if(!path) {
			string path;

			Dl_info info;
			if(dladdr(this, &info) != 0) {

				if(info.dli_fname && info.dli_fname[0]) {
					path = info.dli_fname;
				}
			}

			value["path"] = path;

		} else {

			value["path"] = path;

		}

		return value;
	}

 }
