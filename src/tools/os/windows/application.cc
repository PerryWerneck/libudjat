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
 #include <udjat/tools/application.h>
 #include <errno.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Application::Name::Name(bool with_path) {

		char *ptr;
		TCHAR filename[MAX_PATH];

		if(!GetModuleFileName(NULL, filename, MAX_PATH ) ) {
			throw runtime_error("Can't get module filename");
		}

		// Remove extension.
		ptr = strrchr(filename,'.');
		if(ptr) {
			*ptr = 0;
		}

		if(with_path) {
			assign(filename);
			return;
		}

		ptr = strrchr(filename,'/');
		if(ptr) {
			ptr++;
		} else {
			ptr = filename;
		}

		ptr = strrchr(ptr,'\\');
		if(ptr) {
			ptr++;
		}

		assign(ptr);
	}

	Application::Path::Path() {

		char *ptr;
		TCHAR filename[MAX_PATH];

		if(!GetModuleFileName(NULL, filename, MAX_PATH ) ) {
			throw runtime_error("Can't get module filename");
		}

		ptr = strrchr(filename,'/');
		if(ptr) {
			*(ptr+1) = 0;
		}

		ptr = strrchr(ptr,'\\');
		if(ptr) {
			*(ptr+1) = 0;
		}

		assign(ptr);

	}

	Application::DataDir::DataDir() {

		char *ptr;
		TCHAR filename[MAX_PATH];

		if(!GetModuleFileName(NULL, filename, MAX_PATH ) ) {
			throw runtime_error("Can't get module filename");
		}

		ptr = strrchr(filename,'/');
		if(ptr) {
			*(ptr+1) = 0;
		}

		ptr = strrchr(ptr,'\\');
		if(ptr) {
			*(ptr+1) = 0;
		}

		assign(ptr);

	}


 }
