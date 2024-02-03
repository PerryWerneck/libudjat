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
 #include <udjat/tools/file.h>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/protocol.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	File::Path::Path(const char *v) : std::string{v ? v : ""} {
		expand();
	}

	File::Path::Path(const char *v, size_t s) : std::string{v,s} {
		expand();
	}

	File::Path::operator bool() const noexcept {
		if(empty()) {
			return false;
		}
		return (access(c_str(), R_OK) == 0);
	}

	String File::Path::load() const {
		return File::Text{c_str()}.c_str();
	}

	bool File::Path::find(const char *name, bool recursive) {

		return for_each([this,name](const File::Path &path){
			debug("Testing ",path.c_str());
			if(path.match(name)) {
				assign(path);
				debug("Found ",c_str());
				return true;
			}
			return false;
		},recursive);
	}

	bool File::Path::for_each(const char *pattern, const std::function<bool (const File::Path &path)> &call, bool recursive) const {

		debug("pattern=",pattern);

		return for_each([pattern,call](const File::Path &path){
			if(path.match(pattern)) {
				return call(path);
			}
			return false;
		},recursive);

	}

 }

