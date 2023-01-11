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
 #include <udjat/tools/file.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <system_error>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <libgen.h>
 #include <limits.h>

 using namespace std;

 namespace Udjat {

	void File::copy(const char *from, const char *to, const std::function<bool(double current, double total)> &progress, bool replace) {

		debug("from=",from);
		debug("to=",to);

		int src = open(from,O_RDONLY), dst = -1;

		if(src < 0) {
			throw system_error(errno,system_category(),from);
		}

		if(replace) {

			dst = open(to,O_WRONLY|O_CREAT|O_TRUNC,0600);

		} else {

			char path[PATH_MAX];
			strncpy(path,to,PATH_MAX);
			dst = open(dirname(path),O_WRONLY|O_TMPFILE,0600);

		}

		if(dst < 0) {
			int err = errno;
			::close(src);
			throw system_error(err,system_category(),to);
		}

		struct stat st;
		try {

			if(fstat(src, &st) == -1) {
				throw system_error(errno,system_category(),from);
			}

			size_t buflen = (st.st_blksize * 2);
			char buffer[buflen];

			size_t saved = 0;
			progress(0,0);
			while(saved < (size_t) st.st_size) {

				ssize_t bytes = ::read(src,buffer,buflen);
				if(bytes == 0) {
					clog << "Unexpected EOF reading from " << from << endl;
					break;
				}

				if(bytes < 0) {
					throw system_error(errno,system_category(),from);
				}

				if(::write(dst,buffer,bytes) != bytes) {
					throw system_error(errno,system_category(),to);
				}

				saved += bytes;

				if(!progress((double) saved,(double) st.st_size)) {
					throw system_error(ECANCELED,system_category(),to);
				}
			}
			progress((double) st.st_size,(double) st.st_size);

		} catch(...) {

			::close(src);
			::close(dst);

			throw;

		}

		::close(src);

		// Setup owners and permitions of destination file.
		fchmod(dst,st.st_mode);
		fchown(dst,st.st_uid,st.st_gid);

		if(!replace) {
			File::link(dst,to,false);
		}

		::close(dst);
	}

 }

