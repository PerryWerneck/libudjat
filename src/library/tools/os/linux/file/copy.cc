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

	static void do_copy(int from, int to, const std::function<bool(double current, double total)> &progress) {

		struct stat st;
		if(fstat(from, &st) == -1) {
			throw system_error(errno,system_category());
		}

		size_t buflen = (st.st_blksize * 2);
		char buffer[buflen];

		size_t saved = 0;

		if(progress(0,0)) {
			throw system_error(ECANCELED,system_category());
		}

		while(saved < (size_t) st.st_size) {

			ssize_t bytes = ::read(from,buffer,buflen);
			if(bytes == 0) {
				clog << "Unexpected EOF reading from input file" << endl;
				break;
			}

			if(bytes < 0) {
				throw system_error(errno,system_category());
			}

			if(::write(to,buffer,bytes) != bytes) {
				throw system_error(errno,system_category());
			}

			saved += bytes;

			if(progress((double) saved,(double) st.st_size)) {
				throw system_error(ECANCELED,system_category());
			}
		}

		// Setup owners and permitions of destination file.
		if(fchmod(to,st.st_mode)) {
			Logger::String{"Error '",strerror(errno),"' changing mode of copied file"}.warning();
		}
		
		if(fchown(to,st.st_uid,st.st_gid)) {
			Logger::String{"Error '",strerror(errno),"' changing owner of copied file"}.warning();
		}

		if(progress((double) st.st_size,(double) st.st_size)) {
			throw system_error(ECANCELED,system_category());
		}

	}

	void File::copy(const char *from, const char *to, const std::function<bool(double current, double total)> &progress, bool replace) {

		debug("from=",from);
		debug("to=",to);

		int src = open(from,O_RDONLY), dst = -1;

		if(src < 0) {
			int err = errno;
			cerr << PACKAGE_NAME "\tCant open '" << from << "': " << strerror(err) << endl;
			throw system_error(err,system_category(),from);
		}

		{
			char path[PATH_MAX];
			strncpy(path,to,PATH_MAX);
			dst = open(dirname(path),O_WRONLY|O_TMPFILE,0600);
		}

		if(dst < 0) {

			if(errno != ENOTSUP || !replace) {
				auto err = errno;
				::close(src);
				cerr << PACKAGE_NAME "\tCant create temp file for '" << from << "': " << strerror(err) << endl;
				throw system_error(err,system_category(),to);
			}

			dst = open(to,O_CREAT|O_TRUNC|O_WRONLY,0644);
			if(dst < 0) {
				auto err = errno;
				::close(src);
				cerr << PACKAGE_NAME "\tCant create '" << to << "': " << strerror(err) << endl;
				throw system_error(err,system_category(),to);
			}

			try {

				do_copy(src,dst,progress);

			} catch(...) {

				::close(src);
				::close(dst);
				throw;

			}

			::close(src);
			::close(dst);

		} else {

			try {

				do_copy(src,dst,progress);

			} catch(...) {

				::close(src);
				::close(dst);

				throw;

			}

			::close(src);

			File::move(dst,to,replace);
			::close(dst);

		}

	}

 }

