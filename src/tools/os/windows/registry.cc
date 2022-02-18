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
		LSTATUS rc;

		if(write) {

			rc = RegCreateKeyEx(
							hParent,
							TEXT(path.c_str()),
							0,
							NULL,
							REG_OPTION_NON_VOLATILE,
							KEY_ALL_ACCESS,
							NULL,
							&hKey,
							NULL
			);

			if(rc != ERROR_SUCCESS) {
				throw Win32::Exception( (string{"Can't open registry key at '"} + path + "'").c_str(),rc);
			}

		} else {

			rc = RegOpenKeyEx(
					hParent,
					path.c_str(),
					0,
					KEY_READ,
					&hKey
			);

		}

		if(rc != ERROR_SUCCESS) {
			hKey = 0;
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

		if(!hKey) {
			return def;
		}

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

	string Win32::Registry::get(HKEY hK, const char *name, const char *def) {

		if(!hK)
			return def;

		static DWORD bufLen = 100;

		DWORD dwRet;

		char * data = new char[bufLen];
		memset(data,0,bufLen);

		DWORD cbData = bufLen -1;
		dwRet = RegQueryValueEx( hK, name, NULL, NULL, (LPBYTE) data, &cbData );

		while( dwRet == ERROR_MORE_DATA ) {

			delete[] data;
			bufLen += 1024;
			data = new char[bufLen];
			memset(data,0,bufLen);

			cbData = bufLen-1;
			dwRet = RegQueryValueEx( hK, name, NULL, NULL, (LPBYTE) data, &cbData );
		}

		string rc;
		if(dwRet == ERROR_SUCCESS) {
			rc.assign(data);
		} else {
			rc.assign(def);
		}

		delete[] data;

		return rc;
	}

	std::string Win32::Registry::get(const char *name, const char *def) const {
		return get(hKey, name, def);
	}

	bool Win32::Registry::hasKey(const char *name) const noexcept {

		if(!this->hKey) {
			return false;
		}

		HKEY hK;
		LSTATUS st = RegOpenKeyEx(
							this->hKey,
							name,
							0,
							KEY_READ,
							&hK
						);

		if(st != ERROR_SUCCESS) {
			return false;
		}

		RegCloseKey(hK);
		return true;
	}

	bool Win32::Registry::hasValue(const char *name) const noexcept {

		if(!this->hKey) {
			return false;
		}

		return RegQueryValueEx( hKey, name, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS;
	}

	bool Win32::Registry::for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {

		if(!this->hKey) {
			return false;
		}

		HKEY hGroup;
		if(RegOpenKeyEx(this->hKey,group,0,KEY_ALL_ACCESS,&hGroup) != ERROR_SUCCESS) {
			return false;
		}

		TCHAR achkey[1024];		// buffer for subkey name
		DWORD cbName = 1024;	// size of name string

		bool rc = true;
		for(DWORD index = 0;RegEnumValue(hGroup,index,achkey,&cbName,NULL,NULL,NULL,NULL) == ERROR_SUCCESS && rc; index++) {
			string keyname(achkey,cbName);
			string value = get(hGroup,keyname.c_str(),"");

#ifdef DEBUG
			cout << "key='" << keyname << "' value='" << value << "'" << endl;
#endif // DEBUG

			call(keyname.c_str(),value.c_str());

			cbName = 1024;
		}

		RegCloseKey(hGroup);
		return rc;
	}

 }
