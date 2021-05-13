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
#include <cstdarg>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Request::Request(const char *p) : Json::Value(Json::objectValue), path(p) {
	}

	Request::Request(const std::string &p) : Json::Value(Json::objectValue), path(p) {
	}

	bool Request::operator ==(const char *key) const noexcept {

		if(path.empty())
			return false;

		return strcasecmp(this->path.c_str(),key) == 0;
	}

	string Request::pop() {

		if(path.empty()) {
			throw system_error(ENOENT,system_category(),"The path is incomplete");
		}

		size_t pos = path.find('/');
		if(pos == string::npos) {
			string rc = path;
			path.clear();
			return rc;
		}

		string rc{path.c_str(),pos};
		path.erase(0,pos+1);

		return rc;
	}

	size_t Request::pop(const char *str, ...) {

		string key = pop();
		size_t index = 0;

		va_list args;
		va_start(args, str);
		while(str) {

			if(!strcasecmp(key.c_str(),str)) {
				va_end(args);
				return index;
			}

			index++;
			str = va_arg(args, const char *);
		}
		va_end(args);
		throw system_error(ENOENT,system_category(),string{"Cant find '"} + key + "' on request");

	}

}

