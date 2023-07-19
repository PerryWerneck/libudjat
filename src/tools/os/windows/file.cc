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
 #include <udjat/udjat/defs.h
 #include <udjat/tools/udjat/tools/file.h>
 #include <sys/utime.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

	int File::mtime(const char *filename, time_t timestamp) {

		if(timestamp) {

			_utimbuf ub;
			ub.actime = time(0);
			ub.modtime = timestamp;

			if(_utime(filename,&ub) == -1) {
				int rc = errno;
				Logger::String{"Error '",strerror(rc),"' setting timestamp of '",filename,"'"}.write(Logger::Error,"file");
				return rc = errno;
			}

		}

		return 0;

	}

 }
