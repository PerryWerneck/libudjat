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
 #include <udjat/tools/logger.h>
 #include <cstdio>
 #include <system_error>
 #include <fcntl.h>
 #include <unistd.h>
 #include <limits.h>

 using namespace std;

 namespace Udjat {

	int File::move(const char *tempfile, const char *filename, bool replace) {

		struct stat st;
		if(stat(filename, &st) == -1) {

			if(errno != ENOENT) {
				throw system_error(errno,system_category(),"Error getting file information");
			}

			if(stat(tempfile,&st) == -1) {
				throw system_error(errno,system_category(),tempfile);
			}

		}

		if(linkat(AT_FDCWD, tempfile, AT_FDCWD, filename, AT_SYMLINK_FOLLOW) != 0) {
			//
			// Cant link file
			//
			if(errno != EEXIST) {
				throw system_error(errno,system_category(),filename);
			}

			if(replace) {

				debug("Removing ",filename);
				if(unlink(filename) < 0) {
					Logger::String{"Unable to unlink '",filename,"': ",strerror(errno)," (rc=",errno,")"}.error(PACKAGE_NAME);
				}

			} else {

				// Create backup.
				char bakfile[PATH_MAX+1];
				strncpy(bakfile,filename,PATH_MAX);
				char *ptr = strrchr(bakfile,'.');
				if(ptr) {
					*ptr = 0;
				}
				strncat(bakfile,".bak",PATH_MAX);

				debug("backup=",bakfile);

				if(unlink(bakfile) < 0) {
					Logger::String{"Unable to unlink '",bakfile,"': ",strerror(errno)," (rc=",errno,")"}.error(PACKAGE_NAME);
				}

				if(rename(filename, bakfile) != 0) {
					throw system_error(errno,system_category(),"Cant create backup");
				}

			}

			if(linkat(AT_FDCWD, tempfile, AT_FDCWD, filename, AT_SYMLINK_FOLLOW) != 0) {
				throw system_error(errno,system_category(),filename);
			}

		}

		chmod(filename,st.st_mode);
		chown(filename,st.st_uid,st.st_gid);

		return 0;
	}

	int File::move(int fd, const char *to, bool replace) {
		char tempfile[PATH_MAX];
		snprintf(tempfile, PATH_MAX,  "/proc/self/fd/%d", fd);
		return move(tempfile,to,replace);
	}

 }
