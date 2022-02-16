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

/**
 * @file
 *
 * @brief Implements windows configuration file abstraction.
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/quark.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/logger.h>
 #include <signal.h>
 #include <iostream>
 #include <cstring>

 using namespace std;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 namespace Udjat {

	static recursive_mutex guard;

	class Controller {
	private:
		HKEY hkey = (HKEY) 0;

	public:
		Controller() {

			string regpath{"SOFTWARE\\"};
			regpath += Application::Name();

			DWORD disp = 0;
			LSTATUS status = 0;
#ifdef DEBUG
			status = RegCreateKeyEx(HKEY_CURRENT_USER,regpath.c_str(),0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hkey,&disp);
#else
			status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,regpath.c_str(),0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hkey,&disp);
#endif // DEBUG

			if(status != ERROR_SUCCESS) {
				throw Win32::Exception("Cant open registry",status);
			}

		}

		~Controller() {
			RegCloseKey(hkey);
		}

		DWORD get(const string &group, const string &key, const DWORD def) {

			HKEY hK;
			DWORD disp;

			if(RegCreateKeyEx(this->hkey,group.c_str(),0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hK,&disp) != ERROR_SUCCESS) {
				return def;
			}

			DWORD rc = def;
			unsigned long datatype;
			unsigned long datalen = sizeof(DWORD);

			if(RegQueryValueEx(hK,key.c_str(),NULL,&datatype,(LPBYTE) &rc,&datalen) != ERROR_SUCCESS) {

				rc = def;

		//		RegSetValueEx(hK,key.c_str(),0,REG_DWORD,(const BYTE *) &rc,sizeof(rc));

			} else if(datatype != REG_DWORD) {
				throw runtime_error(string{"Invalid registry data type in "} + group + "/" + key);
			}

			RegCloseKey(hK);

			return rc;
		}

		string get(HKEY hK, const char *name, const char *def) {

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

		string get(const string &group, const string &key, const char *def) {

			HKEY hK;
			DWORD disp;

			if(RegCreateKeyEx(this->hkey,group.c_str(),0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hK,&disp) != ERROR_SUCCESS) {
				return def;
			}

			string rc = get(hK, key.c_str(), def);

			RegCloseKey(hK);

			return rc;

		}

		bool hasGroup(const char *group) {

			HKEY hK;
			LSTATUS st = RegOpenKeyEx(
								this->hkey,
								group,
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

		bool hasKey(const char *group, const char *key) {

			HKEY hGroup, hKey;
			LSTATUS st;
			bool rc = false;

			st = RegOpenKeyEx(
					this->hkey,
					group,
					0,
					KEY_READ,
					&hGroup
				);

			if(st == ERROR_SUCCESS) {

				st = RegOpenKeyEx(
						hGroup,
						key,
						0,
						KEY_READ,
						&hKey
					);

				if(st == ERROR_SUCCESS) {
					RegCloseKey(hKey);
					rc = true;
				}

				RegCloseKey(hGroup);

			}
			return rc;

		}

		bool for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {

			HKEY hGroup;
			if(RegOpenKeyEx(this->hkey,group,0,KEY_READ|KEY_ENUMERATE_SUB_KEYS,&hGroup) != ERROR_SUCCESS) {
				return false;
			}

			TCHAR achkey[1024];		// buffer for subkey name
			DWORD cbName = 1024;	// size of name string
			bool rc = true;
			for(DWORD index = 0;RegEnumKeyEx(hGroup,index,achkey,&cbName,NULL,NULL,NULL,NULL) == ERROR_SUCCESS && rc; index++) {
				string keyname(achkey,cbName);
				string value = get(hGroup,keyname.c_str(),"");

#ifdef DEBUG
				cout << "key='" << keyname << "' value='" << value << "'" << endl;
#endif // DEBUG


				cbName = 1024;
			}

			RegCloseKey(hGroup);
			return rc;
		}

	};

	UDJAT_API bool Config::for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call) {
		return Controller().for_each(group,call);
	}

	UDJAT_API bool Config::hasGroup(const std::string &group) {
		return Controller().hasGroup(group.c_str());
	}

	UDJAT_API bool Config::hasKey(const char *group, const char *key) {
		return Controller().hasKey(group,key);
	}

	UDJAT_API int32_t Config::get(const std::string &group, const std::string &name, const int32_t def) {
		return Controller().get(group,name,def);
	}

	UDJAT_API int64_t Config::get(const std::string &group, const std::string &name, const int64_t def) {
		return Controller().get(group,name,def);
	}

	UDJAT_API uint32_t Config::get(const std::string &group, const std::string &name, const uint32_t def) {
		return Controller().get(group,name,def);
	}

	UDJAT_API uint64_t Config::get(const std::string &group, const std::string &name, const uint64_t def) {
		// FIXME: Implement.
		return def;
	}

	UDJAT_API float Config::get(const std::string &group, const std::string &name, const float def) {
		// FIXME: Implement.
		return def;
	}

	UDJAT_API double Config::get(const std::string &group, const std::string &name, const double def) {
		// FIXME: Implement.
		return def;
	}

	UDJAT_API std::string Config::get(const std::string &group, const std::string &name, const char *def) {
		return Controller().get(group,name,def);
	}

	UDJAT_API std::string Config::get(const std::string &group, const std::string &name, const std::string &def) {
		return Controller().get(group,name,def.c_str());
	}

	UDJAT_API bool Config::get(const std::string &group, const std::string &name, const bool def) {
		return Controller().get(group,name,(DWORD) def) != 0;
	}

 }



