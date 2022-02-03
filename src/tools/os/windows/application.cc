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
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <udjat/win32/registry.h>
 #include <cstring>

#ifdef DEBUG
	#include <iostream>
#endif // DEBUG

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

	const Application::Name & Application::Name::getInstance() {
		static Application::Name instance;
		return instance;
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

		ptr = strrchr(filename,'\\');
		if(ptr) {
			*(ptr+1) = 0;
		}

		assign(filename);

	}

	Application::DataDir::DataDir() : string(Application::Path()) {
	}

	Application::DataFile::DataFile(const char *name) {
		if(name[0] == '/' || (name[0] == '.' && name[1] == '/') || name[0] == '\\' || (name[0] == '.' && name[1] == '\\') || name[1] == ':' ) {
			assign(name);
		} else {
			assign(DataDir());
			append(name);
		}
	}

	Application::LibDir::LibDir() : string(Application::Path()) {
	}

	Application::LibDir::LibDir(const char *subdir) : Application::LibDir() {
		append(subdir);
		append("\\");
		mkdir(c_str());
	}

	Application::SysConfigDir::SysConfigDir() : string(Application::Path()) {
	}

	Application::SysConfigDir::SysConfigDir(const char *subdir) : Application::SysConfigDir() {
		append(subdir);
		append("\\");
		mkdir(c_str());
	}

	Application::LogDir::LogDir() {

		try {

			Win32::Registry registry("log");
			assign(registry.get("path",""));
			if(!empty()) {
				mkdir(c_str());
				return;
			}

		} catch(...) {
			// Ignore errors.
		}

		assign(Application::Path());
		append("\\logs\\");
		mkdir(c_str());
	}

 }
