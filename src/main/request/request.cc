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
 #include <udjat/tools/request.h>
 #include <udjat/tools/string.h>

 namespace Udjat {

	static const char *sanitize(const char *ptr) {

		if(ptr) {
			const char *mark = strstr(ptr,"://");
			if(mark) {
				ptr = mark + 3;
			}

			while(*ptr && *ptr == '/') {
				ptr++;
			}
		}

		return ptr;
	}

	Request::Request(const char *p, HTTP::Method m) : method{m}, path{sanitize(p)} {

		if(path.empty()) {
			throw system_error(EINVAL,system_category(),_("The request path is invalid"));
		}

		debug("Request path is '",path.c_str(),"'");

	}

	Request::Request(const char *path, const char *method) : Request{path,HTTP::MethodFactory(method)} {
	}

	String Request::pop() {

		if(path.empty() || popptr == string::npos) {
			throw system_error(ENODATA,system_category(),_("This request has no arguments"));
		}

		size_t pos = path.find('/',popptr);
		if(pos == string::npos) {
			popptr = pos;
			return path.c_str()+popptr;
		}

		String rc{path.c_str()+popptr,(pos-popptr)};
		popptr = pos+1;
		return rc;

	}

	Request & Request::pop(std::string &value) {

		value = pop();
		return *this;

	}

	Request & Request::pop(int &value) {
		value = stoi(pop());
		return *this;
	}

	Request & Request::pop(unsigned int &value) {
		value = (unsigned int) stoi(pop());
		return *this;
	}

 }

