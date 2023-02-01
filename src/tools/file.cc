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

	UDJAT_API void File::copy(const char *from, const char *to, bool replace) {
		File::copy(from,to,Protocol::Watcher::progress,replace);
	}

	std::string File::save(const char *contents) {

		std::string name = File::Temporary::create();

#ifdef _WIN32
		int out = open(name.c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0644);
#else
		int out = open(name.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
#endif // _WIN32

		if(out < 0) {
			throw system_error(errno,system_category(),"Cant open temporary file");
		}

		try {

			ssize_t len = strlen(contents);
			while(len > 0) {

				ssize_t bytes = write(out,contents,len);

				if(bytes < 0) {
					throw system_error(errno,system_category(),"Error writing temporary file");
				}

				if(bytes < 1) {
					throw runtime_error("Unexpected EOF writing temporary filed");
				}

				len -= bytes;
				contents += bytes;

			}

		} catch(...) {

			::close(out);
			remove(name.c_str());
			throw;

		}

		::close(out);

		return name;
	}

	void File::save(int fd, const char *filename) {

#ifdef _WIN32
		int out = open(filename,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0644);
#else
		int out = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0644);
#endif // _WIN32

		if(out < 0) {
			throw system_error(errno,system_category(),string{"Error opening '"} + filename + "'");
		}

		int in = dup(fd);

#ifndef _WIN32
		fcntl(in,F_SETFL,fcntl(in,F_GETFL,0)|O_RDWR);
#endif // _WIN32

		try {

			if(lseek(in,0,SEEK_SET) == (off_t) -1) {
				throw system_error(errno,system_category(),string{"Error positioning '"} + filename + "'");
			}

			char buffer[4096];

			ssize_t bytes = read(in,buffer,4096);
			while(bytes != 0) {

				if(bytes < 0) {
					throw system_error(errno,system_category(),string{"Error reading source while saving '"} + filename + "'");
				}

				if(write(out,buffer,bytes) != bytes) {
					throw system_error(errno,system_category(),string{"Error saving '"} + filename + "'");
				}

				bytes = read(in,buffer,4096);
			}

		} catch(...) {

			::close(in);
			::close(out);
			throw;

		}

		::close(in);
		::close(out);

	}

 }

