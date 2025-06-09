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

 /**
  * @brief Linux convenience 'file:' methods.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/file.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <utime.h>
 #include <udjat/tools/logger.h>
 #include <system_error>

 using namespace std;

 namespace Udjat {

	int File::mtime(const char *filename, time_t timestamp) {

		if(timestamp) {

			utimbuf ub;
			ub.actime = time(0);
			ub.modtime = timestamp;

			if(utime(filename,&ub) == -1) {
				int rc = errno;
				Logger::String{"Error '",strerror(rc),"' setting timestamp of '",filename,"'"}.write(Logger::Error,"file");
				return errno = rc;
			}

		}

		return 0;

	}

	bool File::outdated(const char *filename, time_t max_age) {

		if(!max_age) {
			return true; // No max age, so its allways outdated.
		}

		struct stat st;

		if(stat(filename,&st) == -1) {
			if(errno == ENOENT) {
				debug(filename," does not exist, so it's outdated");
				return true; // File does not exist, so it's outdated.
			}
			throw system_error(errno,system_category(),filename);
		}

		time_t age = time(0) - st.st_mtime;
		if(age > max_age) {
			debug(filename," is outdated, age: ",age," seconds");
			return true; // File is outdated.
		}

		debug(filename," is not outdated, age: ",age," seconds");
		return false; // File is not outdated.

	}	


 }
