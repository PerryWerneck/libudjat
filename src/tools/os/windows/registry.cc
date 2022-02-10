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

	HKEY Win32::Registry::open(HKEY hParent, const char *p, bool write) {

		HKEY hKey = NULL;
		string path;

		if(!(p && *p)) {
			path = "SOFTWARE\\";
			path += Application::Name();
		} else if(p[0] != '\\' && p[0] != '/') {
			path = "SOFTWARE\\";
			path += Application::Name();
			path += "\\";
			path += p;
		} else {
			path = (p+1);
		}

		for(char *ptr = (char *) path.c_str();*ptr;ptr++) {
			if(*ptr == '/') {
				*ptr = '\\';
			}
		}

		// Open registry.
		DWORD dwOptions = KEY_QUERY_VALUE|KEY_READ|KEY_NOTIFY;

		if(write)
			dwOptions |= KEY_ALL_ACCESS;

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

	Win32::Registry::Registry() : hKey(Win32::Registry::open()) {
	}

	Win32::Registry::Registry(HKEY hParent, const char *path, bool write) : hKey(Win32::Registry::open(hParent,path,write)) {
	}

	Win32::Registry::Registry(const char *path, bool write) : Registry(HKEY_LOCAL_MACHINE,path,write) {
	}

	Win32::Registry::~Registry() {
		if(hKey) {
			RegCloseKey(hKey);
			hKey = 0;
		}
	}

	DWORD Win32::Registry::get(const char *name, DWORD def) const {

		DWORD value = def;
		unsigned long datatype;
		unsigned long datalen = sizeof(DWORD);

		if(RegQueryValueEx(hKey,name,NULL,&datatype,(LPBYTE) &value,&datalen) != ERROR_SUCCESS) {

			value = def;

		} else if(datatype != REG_DWORD) {

			value = def;

		}

		return value;
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

			// Cant log error because the registry is used by logwriter, just use the default.
			rc.assign(def);
		}

		free(buffer);

		return rc;

	}

 }
