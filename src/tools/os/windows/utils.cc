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
 #include <udjat/win32/exception.h>
 #include <string>
 #include <string>

 using namespace std;

 namespace Udjat {

	namespace Win32 {

		/*
		UDJAT_API string getInstallPath() {

			TCHAR path[MAX_PATH];

			if(!GetModuleFileName(NULL, path, MAX_PATH ) ) {
				throw ::Win32::Exception("Can't get application filename");
			}

			char *ptr = strrchr((const char *) path,'\\');
			if(ptr)
				*(ptr+1) = 0;

			return path;

		}

		UDJAT_API string buildFileName(const char *path, ...) {

			string filename = getInstallPath();
			bool sep = false;

			va_list args;
			va_start(args, path);
			while(path) {
				if(sep) {
					filename += "\\";
				}
				filename += path;
				sep = true;
				path = va_arg(args, const char *);
			}
			va_end(args);


		}
		*/

	}

 }
