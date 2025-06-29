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
 
 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	UDJAT_API void File::copy(const char *from, const char *to, bool replace) {
		File::copy(from,to,[](unsigned long long, unsigned long long) { return false; },replace);
	}

	UDJAT_API void File::copy(const char *filename, const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) {

		int fd = open(filename,O_RDONLY);
		if(fd < 0) {
			throw system_error(errno,system_category(),String{"Cant open temporary ",filename});
		}

		struct stat st;
		if(fstat(fd,&st)) {
			throw system_error(errno,system_category(),String{"Cant get length of ",filename});
		}

		unsigned long long offset = 0;
#ifdef _WIN32
		char buffer[512];
#else
		char buffer[st.st_blksize+1];
#endif // _WIN32

		while(offset < (unsigned long long) st.st_size) {

#ifdef _WIN32
			if(lseek(fd, offset, SEEK_SET) < 0) {
				throw system_error(errno,system_category(),"Cant set file offset");
			}
			ssize_t bytes = ::read(fd,buffer,512);
#else
			ssize_t bytes = pread(fd,buffer,st.st_blksize,offset);
#endif // _WIN32
			if(bytes < 0) {
				throw system_error(errno,system_category(),"Cant read from file");
			} else if(bytes == 0) {
				throw runtime_error("Unexpected EOF reading from file");
			}

			writer(offset,st.st_size,buffer,bytes);

			offset += bytes;

		}

		::close(fd);

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

	void File::copy(int from, const char *to) {

#ifdef _WIN32
		int out = open(to,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0644);
#else
		int out = open(to,O_WRONLY|O_CREAT|O_TRUNC,0644);
#endif // _WIN32

		if(out < 0) {
			throw system_error(errno,system_category(),string{"Error opening '"} + to + "'");
		}

		int in = dup(from);

#ifndef _WIN32
		fcntl(in,F_SETFL,fcntl(in,F_GETFL,0)|O_RDWR);
#endif // _WIN32

		try {

			if(lseek(in,0,SEEK_SET) == (off_t) -1) {
				throw system_error(errno,system_category(),string{"Error positioning '"} + to + "'");
			}

			char buffer[4096];

			ssize_t bytes = read(in,buffer,4096);
			while(bytes != 0) {

				if(bytes < 0) {
					throw system_error(errno,system_category(),string{"Error reading source while saving '"} + to + "'");
				}

				if(write(out,buffer,bytes) != bytes) {
					throw system_error(errno,system_category(),string{"Error saving '"} + to + "'");
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

