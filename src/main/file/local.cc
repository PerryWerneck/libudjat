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

 #include "private.h"
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>

 namespace Udjat {

	File::Local::Local(const char *path) : contents(nullptr),length(0) {

		int fd = open(path,O_RDONLY);
		if(fd < 0) {
			throw system_error(errno, system_category(), (string{"Can't open '"} + path + "'"));
		}

		try {
			struct stat st;
			if(fstat(fd, &st)) {
				throw system_error(errno, system_category(), (string{"Can't get size of '"} + path + "'"));
			}

			length = st.st_size;


			if(length) {

				// Can't get the file length, use mmap.
				this->mapped = true;
				this->contents = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);

				if(contents == MAP_FAILED) {

					if(errno == ENODEV) {
						throw runtime_error("The underlying filesystem of the specified file does not support memory mapping.");
					}

					throw system_error(errno, system_category(), (string{"Can't map '"} + path + "'"));
				}

			} else {

				// No file length, uses the 'normal way'.
				mapped = false;
				ssize_t in;
				size_t szBuffer = 4096;
				length = 0;
				contents = malloc(szBuffer);

				char * ptr = (char *) contents;
				while( (in = read(fd,ptr,szBuffer - length)) != 0) {

					if(in < 0) {
						throw system_error(errno, system_category(), (string{"Can't read '"} + path + "'"));
					}

					if(in > 0) {
						ptr += in;
						length += in;
						if(length >= (szBuffer - 20)) {
							szBuffer += 4096;
							contents = realloc(contents,szBuffer);
							ptr = (char *) contents+length;
						}
					}

				}

				contents = (char *) realloc(contents,length+1);
				((char *) contents)[length] = 0;

			}


		} catch(...) {

			close(fd);
			throw;

		}

	}

	File::Local::Local(const File::Agent &agent) : Local(agent.c_str()) {
	}

	File::Local::~Local() {
		if(mapped) {
			munmap(contents,length);
		} else {
			free(contents);
		}
	}

	void File::Local::forEach(std::function<void (const string &line)> call) {
		forEach(c_str(),call);
	}

	void File::Local::forEach(const char *from, std::function<void (const string &line)> call) {
		while(from) {
			const char *to = strchr(from,'\n');
			if(to) {
				call(string(from,to-from));
				from = to+1;
			} else {
				call(string(from));
				break;
			}
		}

	}

 }
