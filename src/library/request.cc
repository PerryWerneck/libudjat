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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/intl.h>
 #include <cstdarg>

 namespace Udjat {

	Request::Request(const char *path) : reqpath{path} {

		if(reqpath && *reqpath) {
			pop(reqpath,apiver);
		} else {
			reqpath = "";
		}
		
		rewind();

		debug("Build request for '",reqpath,"'");
	}

	Request::~Request() {
	}

	Authentication::Role Request::role() const noexcept {
		if(auth) {
			return auth->role();
		}
		return Authentication::None;
	}

	MimeType Request::mimetype() const noexcept {
		return MimeType::text;
	}

	HTTP::Method Request::method() const noexcept {
		return HTTP::Get;
	}

	void Request::logger(Logger::Level level, const char *domain, const char *text) const noexcept {
		Logger::String{text}.write(level,domain);
	}

	bool Request::pop(std::string &out, const char * &path) noexcept {

		if(path[0] != '/' || path[1] == 0) {
			debug("Rejecting invalid or empty path");
			return false;
		}

		const char *ptr = strchr(path+1,'/');
		if(ptr) {

			size_t len = (path+1) - ptr;
			out = string{(ptr+1),len};

		} else {
			out = string{(ptr+1)};
		}

		path += out.size();

		return true;

	}

	bool Request::pop(const char *prefix, const char * &path) noexcept {

		if(path[0] != '/' || path[1] == 0) {
			debug("Rejecting invalid or empty path");
			return false;
		}

		size_t length = strlen(prefix);

		// debug("path= '",(path+1),"'");
		// debug("prefix= '",prefix,"'");
		// debug(strncasecmp(path+1,prefix,length));
		// debug(path+(length+1));

		if(strncasecmp(path+1,prefix,length) || (path[length+1] && path[length+1] != '/')) {
			debug("Path '",path,"' doesnt match '",prefix,"'");
			return false;
		}

		debug("Path '",path,"' match '",prefix,"'");

		path += length+1;
		return true;
	}

	bool Request::pop(const char * &path, unsigned int &apiver) {
		
		apiver = 0;
		if(!pop("api",path)) {
			return false;
		}

		if(path[0] == '/' && isdigit(path[1])) {
			path++;
			while(*path && *path != '/') {
				if(isdigit(*path)) {
					apiver += (*path - '0');
				} else if(*path == '.') {
					apiver *= 100;
				} else {
					throw runtime_error(_("Invalid or unexpected API version"));
				}
				path++;
			}
		} else {
			apiver = 1000000;
		}
		
		return true;
	}

	bool Request::cached(const TimeStamp &) const {
		return false;
	}

	const char * Request::query(const char *def) const {
		return def;
	}

	const char * Request::username() const {
		if(auth) {
			return auth->c_str();
		} 
		return "";
	}

	bool Request::for_each(const std::function<bool(const char *name, const char *value)> &call) const {
		return Variant::for_each([call](const char *name, const Value &value){
			if(value.isString()) {
				return call(name,value.c_str());
			}
			return false;	
		});
	}

	bool Request::get_property(const char *key, Udjat::Variant &value) const {

		if(!strcasecmp(key,"path")) {
			value = path();
			return true;
		}

		if(!strcasecmp(key,"apiver")) {
			value = apiver;
			return true;
		}

		if(!strcasecmp(key,"username")) {
			value = username();
			return true;
		}

		return Variant::get_property(key,value);
	}

	const char * Request::header(const char *name) const noexcept {
		Logger::String{"Returning empty value for header '",name,"'"}.trace();
		return "";
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

	int Request::select(const char *value, ...) noexcept {

		if(argptr[0] == 0 || argptr[1] == 0) {
			return -ENODATA;
		}

		if(argptr[0] != '/') {
			return -EINVAL;
		}

		int rc = -ENOENT;
		int index = 0;
		va_list args;
		va_start(args, value);

		while(value) {
			if(pop(value,argptr)) {
				rc = index;
				break;
			}
			index++;
			value = va_arg(args, const char *);
		}

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

 }

