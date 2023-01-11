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
 #include <udjat/tools/application.h>
 #include <udjat/win32/exception.h>

 using namespace std;

 namespace Udjat {

	UDJAT_API int File::move(const char *from, const char *to, bool replace) {

		debug("from=",from);
		debug("to=",to);

		if(replace) {

			// No backup, just move
			DeleteFile(to);

		} else {

			// Create backup.
			char bakfile[PATH_MAX+1];
			strncpy(bakfile,to,PATH_MAX);
			char *ptr = strrchr(bakfile,'.');
			if(ptr) {
				*ptr = 0;
			}
			strncat(bakfile,".bak",PATH_MAX);

			DeleteFile(bakfile);
			MoveFile(to,bakfile);

		}

		if(MoveFile(from,to)) {
			return 0;
		}

		throw Win32::Exception(to);

	}

	UDJAT_API void File::copy(const char *from, const char *to, const std::function<bool(double current, double total)> &progress, bool replace) {

		char tempname[PATH_MAX+1];
		memset(tempname,0,PATH_MAX+1);

		debug("from=",from);
		debug("to=",to);

		int src = open(from,O_RDONLY|O_BINARY), dst = -1;

		if(src < 0) {
			throw system_error(errno,system_category(),from);
		}

		{
			char path[PATH_MAX+1];
			strncpy(path,to,PATH_MAX);

			char *p0 = strrchr(path,'/');
			char *p1 = strrchr(path,'\\');

			if(p0 && !p1) {
				*p0 = 0;
			} else if(p1 && !p0) {
				*p1 = 0;
			} else if(p0 && p1) {
				*(p0 < p1 ? p0 : p1) = 0;
			} else {
				strncpy(path,".",PATH_MAX);
			}

			debug("dirname=",path);

			if(GetTempFileName(path,TEXT(Application::Name().c_str()),0,tempname) == 0) {
				throw Win32::Exception("Unable to create tempfile");
			}

			debug("Tempname=",tempname);
			dst = open(tempname,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0644);

		}

		if(dst < 0) {
			int err = errno;
			::close(src);
			DeleteFile(tempname);
			throw system_error(err,system_category(),to);
		}

		struct stat st;
		try {

			if(fstat(src, &st) == -1) {
				throw system_error(errno,system_category(),from);
			}

			#define buflen 1024
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
			DeleteFile(tempname);

			throw;

		}

		::close(src);
		::close(dst);

		File::move(tempname,to,replace);

	}

 }

