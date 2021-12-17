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
 #include <udjat/win32/registry.h>
 #include <udjat/win32/exception.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	static HKEY open(HKEY hParent, const char *p, bool write) {

		HKEY hKey = 0;
		string path;

		if(!hParent) {

			hParent = HKEY_LOCAL_MACHINE;

			if(!(p && *p)) {

				path = "SOFTWARE\\";
				path += Application::Name();

			} if(p[0] != '\\') {

				path = "SOFTWARE\\";
				path += Application::Name();
				path += "\\";
				path += p;

			} else {

				path = (p+1);

			}

		} else if(!(p && *p)) {

			throw runtime_error("Invalid registry path");

		}

		cout << "5a" << endl;
		cout << "PATH=" << path << endl;

		// Open registry.
		DWORD dwOptions = KEY_QUERY_VALUE|KEY_READ|KEY_NOTIFY;

		if(write)
			dwOptions |= KEY_ALL_ACCESS;


		for(char *ptr = (char *) path.c_str();*ptr;ptr++) {
			if(*ptr == '/') {
				*ptr = '\\';
			}
		}

		cout << "PATH: '" << path << "' ";

		LSTATUS rc = RegCreateKeyEx(
							hParent,
							TEXT(path.c_str()),
							0,
							NULL,
							REG_OPTION_NON_VOLATILE,
							dwOptions,
							NULL,
							&hKey,
							NULL
						);

		if(rc != ERROR_SUCCESS) {
			throw Win32::Exception( (string{"Can't open registry key at '"} + path + "'").c_str(),rc);
		}

		return hKey;

	}

	static void close(HKEY hKey) {
		if(hKey) {
			RegCloseKey(hKey);
			hKey = 0;
		}
	}

	Win32::Registry::Registry(HKEY hParent, const char *path, bool write) : hKey(open(hParent,path,write)) {
	}

	Win32::Registry::Registry(const char *path, bool write) : hKey(open(0,path,write)) {
	}

	Win32::Registry::~Registry() {
		Udjat::close(hKey);
	}

	std::string Win32::Registry::get(const char *name, const char *def) const {

		DWORD		  cbData	= 4096;
		char		* buffer	= (char *) malloc(cbData+1);
		std::string	  rc;

		DWORD dwRet = RegQueryValueEx(	hKey,
										name,
										NULL,
										NULL,
										(LPBYTE) buffer,
										&cbData );

		if(dwRet == ERROR_MORE_DATA) {
			buffer = (char *) realloc(buffer,cbData+1);
			dwRet = RegQueryValueEx(	hKey,
										name,
										NULL,
										NULL,
										(LPBYTE) buffer,
										&cbData );

		}

		if(dwRet == ERROR_SUCCESS) {
			buffer[cbData] = 0;
			rc.assign(buffer);
		} else {

			if(dwRet != ERROR_FILE_NOT_FOUND) {
				clog << "Can't read " << name << " from registry, assuming \"" << def << "\" (error code was " << dwRet << ")" << endl;
			}

			rc.assign(def);
		}

		free(buffer);

		return rc;

	}

 }
