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

	Abstract::File::~File() {
		if(fd > 0) {
			::close(fd);
		}
	}

	ssize_t Abstract::File::write(const void *contents, size_t length) {

		ssize_t rc = length;

		while(length) {

			ssize_t bytes = ::write(fd, contents, length);
			if(bytes < 1) {
				throw system_error(errno,system_category(),"Cant write to temporary file");
			}

			length -= bytes;
			contents = (void *) (((uint8_t *) contents) + bytes);

		}

		return rc;

	}

	ssize_t Abstract::File::read(void *contents, size_t length, bool required) {

		ssize_t complete = 0;

		do {

			ssize_t bytes = ::read(fd,contents,length);
			if(bytes < 0) {
				throw system_error(errno,system_category(),"Cant read from temporary file");
			} else if(bytes == 0) {
				break;
			}
			complete += bytes;

		} while(required && ((size_t) complete) < length);

		return complete;

	}


 }

