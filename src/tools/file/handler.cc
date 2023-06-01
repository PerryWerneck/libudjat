/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

	File::Handler::~Handler() {
		if(fd > 0) {
			::close(fd);
		}
	}

	ssize_t File::Handler::write(unsigned long long offset, const void *contents, size_t length) {

		ssize_t rc = length;

		while(length) {

			ssize_t bytes = ::pwrite(fd, contents, length, offset);
			if(bytes < 1) {
				throw system_error(errno,system_category(),"Cant write to file");
			}

			length -= bytes;
			offset += bytes;

			contents = (void *) (((uint8_t *) contents) + bytes);

		}

		return rc;

	}

	ssize_t File::Handler::write(const void *contents, size_t length) {

		ssize_t rc = length;

		while(length) {

			ssize_t bytes = ::write(fd, contents, length);
			if(bytes < 1) {
				throw system_error(errno,system_category(),"Cant write to file");
			}

			length -= bytes;
			contents = (void *) (((uint8_t *) contents) + bytes);

		}

		return rc;

	}

	ssize_t File::Handler::read(void *contents, size_t length, bool required) {

		ssize_t complete = 0;

		do {

			ssize_t bytes = ::read(fd,contents,length);
			if(bytes < 0) {
				throw system_error(errno,system_category(),"Cant read from file");
			} else if(bytes == 0) {
				break;
			}
			complete += bytes;

		} while(required && ((size_t) complete) < length);

		return complete;

	}

	ssize_t File::Handler::read(unsigned long long offset, void *contents, size_t length, bool required) {

		ssize_t complete = 0;

		do {

			ssize_t bytes = ::pread(fd,contents,length,offset);
			if(bytes < 0) {
				throw system_error(errno,system_category(),"Cant read from file");
			} else if(bytes == 0) {
				break;
			}
			complete += bytes;

		} while(required && ((size_t) complete) < length);

		return complete;

	}
	void File::Handler::save(const std::function<void(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &write) const {

		struct stat st;
		if(fstat(fd,&st)) {
			throw system_error(errno,system_category(),"Cant get file length");
		}

		unsigned long long offset = 0;
		char buffer[st.st_blksize+1];

		while(offset < st.st_size) {

			ssize_t bytes = pread(fd,buffer,st.st_blksize,offset);
			if(bytes < 0) {
				throw system_error(errno,system_category(),"Cant read from file");
			} else if(bytes == 0) {
				throw runtime_error("Unexpected EOF reading from file");
			}

			write(offset,st.st_size,buffer,bytes);

			offset += bytes;

		}

	}

 }

