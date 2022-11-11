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

	static const char *regpath = "SOFTWARE";

	static string PathFactory(const char *p) noexcept {

		string path{regpath};

		if(!(p && *p)) {

			path += "\\";
			path += Application::Name();

		} else if(p[0] != '\\' && p[0] != '/') {

			path += "\\";
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

		return path;

	}

	HKEY Win32::Registry::open(HKEY hParent, const char *p, bool write) {

		HKEY hKey = NULL;
		string path{PathFactory(p)};

		// Open registry.
		LSTATUS rc;

		if(write) {

			// Write access requested, throw exception if failed.

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

			// Read only access, open and use defaults if failed.

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

	Win32::Registry::Registry(bool write) : hKey(Win32::Registry::open(HKEY_LOCAL_MACHINE,nullptr,write)) {
	}

	Win32::Registry::Registry(const char *path, bool write) : hKey(Win32::Registry::open(HKEY_LOCAL_MACHINE,path,write)) {
	}

	Win32::Registry::~Registry() {
		if(hKey) {
			RegCloseKey(hKey);
			hKey = 0;
		}
	}

	void Win32::Registry::setRoot(const char *path) {
		regpath = path;
	}

	void Win32::Registry::remove(const char *keyname) {
		auto rc = RegDeleteTree(hKey,keyname);
		if(rc != ERROR_SUCCESS && rc != ERROR_FILE_NOT_FOUND) {
			throw Win32::Exception("Cant delete application registry",rc);
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

	UINT64 Win32::Registry::get(const char *name, UINT64 def) const {

		if(!hKey) {
			return def;
		}

		UINT64 value = 0;
		unsigned long datatype;
		unsigned long datalen = sizeof(UINT64);

		if(RegQueryValueEx(hKey,name,NULL,&datatype,(LPBYTE) &value,&datalen) != ERROR_SUCCESS) {

			value = def;

		} else if(datatype == REG_DWORD) {

			const DWORD *dw = ((DWORD *) &def);
			return (UINT64) *dw;

		} else if(datatype != REG_QWORD) {

			value = def;

		}

		return value;
	}

	void * Win32::Registry::get(const char *name, void *ptr, size_t len) {
		return get(hKey,name,ptr,len);
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

	void * Win32::Registry::get(HKEY hKey, const char *name, void *ptr, size_t len) {

		uint8_t datablock[len+1];
		memset(datablock,0,len+1);

		DWORD datalen = len;
		DWORD datatype = 0;

		if(RegQueryValueEx(hKey,name,NULL,&datatype,(LPBYTE) datablock,&datalen) != ERROR_SUCCESS) {

			return nullptr;

		} else if(datatype != REG_BINARY) {

			clog << "win32\tNon binary data on registry value '" << name << "', was expecting a binary block with " << len << " bytes" << endl;

		} else if(datalen == len) {

			memcpy(ptr,datablock,len);
			return ptr;

		} else {

			clog << "win32\tUnexpected length " << datalen << " reading value '" << name << "' from registry (was expecting " << len << ")" << endl;

		}

		return nullptr;
	}

	void Win32::Registry::set(HKEY hK, const char *name, const char *value) {

		DWORD dwRet = RegSetValueEx(
							hK,
							(LPCSTR) name,
							0,
							REG_SZ,
							(const BYTE *) value,
							(DWORD) strlen(value)
					);

		if(dwRet != ERROR_SUCCESS) {
			throw Win32::Exception(dwRet);
		}

	}

	void Win32::Registry::set(HKEY hK, const char *name, const void *ptr, size_t len) {

		DWORD dwRet = RegSetValueEx(
							hK,
							(LPCSTR) name,
							0,
							REG_BINARY,
							(const BYTE *) ptr,
							(DWORD) len
					);

		if(dwRet != ERROR_SUCCESS) {
			throw Win32::Exception(dwRet);
		}

	}

	std::string Win32::Registry::get(const char *name, const char *def) const {
		return get(hKey, name, def);
	}

	void Win32::Registry::set(const char *name, const char *value) {
		set(hKey, name, value);
	}

	void Win32::Registry::set(const char *name, const void *ptr, size_t length) {
		set(hKey, name, ptr, length);
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
