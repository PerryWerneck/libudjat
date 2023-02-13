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
 #include <private/request.h>
 #include <cstring>
 #include <cstdarg>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>

 namespace Udjat {

	bool Request::operator ==(const char *key) const noexcept {

		if(method.empty())
			return false;

		return strcasecmp(this->method.c_str(),key) == 0;
	}

	string Request::pop() {

		if(empty()) {
			throw system_error(ENODATA,system_category(),_("This request has no arguments"));
		}

		size_t pos = path.find('/');
		if(pos == string::npos) {
			string rc{path};
			path.clear();
			return rc;
		}

		string rc{path.c_str(),pos};
		path.erase(0,pos+1);

		return rc;

	}

	int Request::pop(const char *str, ...) {

		string key = pop();
		int index = 0;

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

		return -1;

	}

	const std::string Request::getAction() {
		string rc;
		pop(rc);
		return rc;
	}

	int Request::select(const char *str, ...) {

		string key = getAction();
		int index = 0;

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

		return -1;

	}

	Request & Request::pop(std::string &value) {

		value = pop();
		return *this;

	}

	Request & Request::pop(int &value) {
		string v;
		pop(v);
		value = stoi(v);
		return *this;
	}

	Request & Request::pop(unsigned int &value) {
		string v;
		pop(v);
		value = (unsigned int) stoi(v);
		return *this;
	}

 }

