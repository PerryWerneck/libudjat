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
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/configuration.h>

 namespace Udjat {

	bool Request::cached(const TimeStamp &timestamp) const {
		return false;
	}

	String Request::getProperty(const char *, const char *def) const {
		return def;
	}

	const char * Request::query(const char *def) const {
		return def;
	}

	String Request::operator[](const char *name) const {

		String value{getArgument(name,"")};
		if(!value.empty()) {
			return value;
		}
		return getProperty(name);

	}

	/// @brief Get argument.
	/// @param name The argument name
	/// @param def The default value.
	String Request::getArgument(const char *name, const char *def) const {

		String value{def};

		const char *query = this->query();
		if(query && *query) {

			size_t szName = strlen(name);
			String{query}.for_each("&",[&value,szName,name](const String &v){

				if(v.size() > szName) {

					if(v[szName] == '=' && strncasecmp(v.c_str(),name,szName) == 0) {
						value = v.c_str()+szName+1;
						value.strip();
						return true;
					}
				}

				return false;

			});

		}

		return value;
	}

	String Request::getProperty(int ix, const char *def) const {

		debug("reqpath='",reqpath,"' ix=",ix," def='",def,"'");

		if(*reqpath != '/') {
			throw system_error(EINVAL,system_category(),"Request should start with '/' to use indexed parameter");
		}

		if(ix < 1) {
			throw system_error(EINVAL,system_category(),"Request argument index should start with '1'");
		}

		const char *ptr = reqpath+1;
		while(*ptr) {

			const char *next = strchr(ptr,'/');
			if(!next) {
				if(ix == 1) {
					return ptr;
				}
				break;
			}

			if(!--ix) {
				return String{ptr,(size_t) (next-ptr)};
			}

			ptr = next+1;
		}

		return def;
	}

	String Request::pop() {

		if(!argptr) {
			rewind();
			if(*argptr != '/') {
				throw system_error(EINVAL,system_category(),"Request should start with '/' to pop values");
			}
			argptr++;
		}

		if(!*argptr) {
			return "";
		}

		const char *next = strchr(argptr,'/');
		if(!next) {
			string rc{argptr};
			argptr = "";
			return rc;
		}

		string rc{argptr,(size_t) (next-argptr)};
		argptr = next+1;

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

	const char * Request::c_str() const noexcept {
		Logger::String{"Processing request with no path"}.warning("request");
		return "";
	}

 }

