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
			this->contents = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);

			if(contents == MAP_FAILED) {

				if(errno == ENODEV) {
					throw runtime_error("The underlying filesystem of the specified file does not support memory mapping.");
				}

				throw system_error(errno, system_category(), (string{"Can't map '"} + path + "'"));
			}

		} catch(...) {

			close(fd);
			throw;

		}

	}

	File::Local::Local(const File::Agent &agent) : Local(agent.getName()) {
	}

	File::Local::~Local() {
		munmap(contents,length);
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
