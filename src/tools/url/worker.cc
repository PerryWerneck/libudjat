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
 #include <cstring>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 namespace Udjat {

	Protocol::Worker & Protocol::Worker::credentials(const char UDJAT_UNUSED(*user), const char UDJAT_UNUSED(*passwd)) {
		throw system_error(ENOTSUP,system_category(),"No credentials support on selected worker");
	}

	Protocol::Header & Protocol::Worker::header(const char UDJAT_UNUSED(*name)) {
		throw system_error(ENOTSUP,system_category(),string{"The selected worker was unable do create header '"} + name + "'");
	}

	static const std::function<bool(double current, double total)> dummy_progress([](double UDJAT_UNUSED(current), double UDJAT_UNUSED(total)) {
		return true;
	});

	String Protocol::Worker::get() {
		return get(dummy_progress);
	}

	bool Protocol::Worker::save(const char *filename) {
		return save(filename, dummy_progress);
	}

	bool Protocol::Worker::save(const char *filename, const std::function<bool(double current, double total)> &progress) {

		String text = get(progress);

		size_t length = text.size();
		int fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0777);
		if(fd < 0) {
			throw system_error(errno,system_category(),filename);
		}

		const char *ptr = text.c_str();
		while(length) {
			ssize_t bytes = write(fd,ptr,length);
			if(bytes < 0) {
				int err = errno;
				::close(fd);
				throw system_error(err,system_category(),filename);
			} else if(bytes == 0) {
				::close(fd);
				throw runtime_error(string{"Unexpected error writing "} + filename);
			}
			ptr += bytes;
			length -= bytes;
		}
		::close(fd);

		return true;
	}

 }


