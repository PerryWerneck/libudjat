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

	bool Request::cached(const TimeStamp &) const {
		return false;
	}

	const char * Request::query(const char *def) const {
		return def;
	}

	bool Request::authenticated() const noexcept {
		return false;
	}

	String Request::operator[](const char *name) const {

		String rc;

		if(isdigit(*name)) {
			int ix = atoi(name);
			if(ix > 0) {
				if(getProperty((size_t) ix, rc)) {
					return rc;
				}
			}
		}

		for_each([&rc,name](const char *n, const char *v) {
			if(!strcasecmp(n,name)) {
				rc = v;
				return true;
			}
			return false;
		});

		return rc;
	}

	bool Request::for_each(const std::function<bool(const char *name, const char *value)> &call) const {

		// Split query arguments.
		const char *args = query();
		if(args && *args) {

			return String{args}.for_each("&",[&call](const String &arg) {

				const char *n = arg.c_str();
				const char *v = strchr(n,'=');

				if(v && call(string{n,(size_t)(v-n)}.c_str(),v+1)) {
					return true;
				}

				return false;

			});
		}
		return false;

	}

	bool Request::for_each(const std::function<bool(const char *name, const Value &value)> &call) const {

		return for_each([call](const char *name, const char *value){

			auto vptr = Value::Factory(value);

			if(call(name,*vptr)) {
				return true;
			}

			return false;
		});

		return false;
	}

	bool Request::getProperty(const char *key, Udjat::Value &value) const {
		return for_each([key,&value](const char *n, const Udjat::Value &v){

			if(!strcasecmp(n,key)) {
				value = v;
				return true;
			}

			return false;
		});
	}

	bool Request::getProperty(const char *key, std::string &value) const {

		if(isdigit(*key)) {
			int ix = atoi(key);
			if(ix > 0) {
				if(getProperty((size_t) ix, value)) {
					return true;
				}
			}
		}

		return for_each([key,&value](const char *n, const char *v){
			if(!strcasecmp(n,key)) {
				value = v;
				return true;
			}
			return false;
		});
	}

	bool Request::getProperty(size_t ix, std::string &value) const {

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
					value = ptr;
					return true;
				}
				break;
			}

			if(!--ix) {
				value = string{ptr,(size_t) (next-ptr)};
				return true;
			}

			ptr = next+1;
		}

		return false;

	}

	bool Request::pop(const char *path) noexcept {

		if(!(path && *path)) {
			return false;
		}

		if(*path == '/') {
			path++;
		}

		if(!argptr) {
			rewind();
		}

		const char *ptr = argptr;
		if(*ptr == '/') {
			ptr++;
		}

		int szPath = strlen(path);
		int szArg = strlen(ptr);

		if(szArg > szPath && ptr[szPath] == '/' && strncasecmp(path,ptr,szPath) == 0) {
			argptr = (ptr + szPath);
			return true;
		}

		return false;
	}

	String Request::pop() {

		if(!argptr) {
			rewind();
		}

		if(*argptr == '/') {
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

	const char * Request::path() const noexcept {
		return argptr ? argptr : reqpath;
	}

	bool Request::operator==(const char *path) const noexcept {

		if(*path == '/') {
			path++;
		}

		const char *arg = argptr ? argptr : reqpath;
		if(*arg == '/') {
			arg++;
		}

		size_t szarg = strlen(arg);
		size_t szpath = strlen(path);

		if(szarg < szpath) {
			return false;
		}

		if(szarg > szpath) {
			return strncasecmp(arg,path,szpath) == 0 && path[szarg] == '/';
		}

		return strcasecmp(arg,path) == 0;
	}

	int Request::select(const char *value, ...) noexcept {

		String action{pop()};

		debug("action='",action,"'");

		va_list args;
		va_start(args, value);
		int rc = action.select(value,args);
		va_end(args);
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

