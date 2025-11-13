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
 #include <udjat/tools/url.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/logger.h>
 #include <private/url.h>
 #include <stdexcept>
 #include <errno.h>

 #ifdef _WIN32
	#include <shlwapi.h>
 #endif

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	FileURLHandler::FileURLHandler(const URL &url) : path{url.path().c_str()} {
		debug("Created FileURLHandler for '",path.c_str(),"'");
	}

	const char * FileURLHandler::c_str() const noexcept {
		return path.c_str();
	}

	int FileURLHandler::perform(const HTTP::Method, const char *, const std::function<bool(uint64_t current, uint64_t total, const void *data, size_t len)> &progress) {

		debug("-----------> Loading '",path.c_str(),"'");

		File::Handler file{path.c_str()};

		size_t block_size = file.block_size();
		char buffer[block_size];

		uint64_t total = file.length();
		uint64_t current = 0;

		while(current < total) {

			size_t len = file.read((void *) buffer,block_size,false);
			if(!len) {
				break;
			}

			if(progress(current,total,buffer,len)) {
				Logger::String{"Loading of '",path.c_str(),"' was canceled"}.trace();
				throw system_error(ECANCELED,system_category());
			}

			current += len;

		}

		return 200;

	}

	int FileURLHandler::test(const HTTP::Method, const char *) {

#ifdef _WIN32
		if(!PathFileExists(path.c_str())) {
			return 404;
		}

		return 200;
#else
		if(access(path.c_str(),R_OK) == 0) {
			return 200;
		}

		if(access(path.c_str(),F_OK) != 0) {
			return 404;
		}

		return 401;
#endif // _WIN32

	}
 
}
