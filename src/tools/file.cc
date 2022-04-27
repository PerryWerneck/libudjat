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

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	void File::copy(const char *from, const char *to) {

#ifdef _WIN32
		int fd = open(from,O_RDONLY|O_BINARY);
#else
		int fd = open(from,O_RDONLY);
#endif // _WIN32

		if(fd < 0) {
			throw system_error(errno,system_category(),string{"Error opening '"} + from + "'");
		}

		try {

			save(fd,to);

		} catch(...) {

			::close(fd);
			throw;

		}

		::close(fd);

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

