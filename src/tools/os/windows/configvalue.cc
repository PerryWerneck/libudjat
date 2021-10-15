/**
 *
 * Copyright (C) <2017> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @file
 *
 * @brief Implements linux configuration file.
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 */

 #include <config.h>
 #include <windows.h>
 #include <udjat/tools/configuration.h>
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

#ifdef DEBUG
	static Quark confname = Quark::getFromStatic("udjat");
#else
	static Quark confname;
#endif // DEBUG

	class Controller {
	private:
		HKEY hkey = (HKEY) 0;

	public:
		Controller() {

			if(!confname) {

				// Get application name
#ifndef DEBUG
				#error More work here!
#endif // DEBUG

			}

			string regpath{"SOFTWARE\\"};
			regpath += confname.c_str();

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

		string get(const string &group, const string &key, const char *def) {

			HKEY hK;
			DWORD disp;

			if(RegCreateKeyEx(this->hkey,group.c_str(),0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hK,&disp) != ERROR_SUCCESS) {
				return def;
			}

			static DWORD bufLen = 100;

			DWORD dwRet;

			char * data = new char[bufLen];
			memset(data,0,bufLen);

			DWORD cbData = bufLen -1;
			dwRet = RegQueryValueEx( hK, key.c_str(), NULL, NULL, (LPBYTE) data, &cbData );

			while( dwRet == ERROR_MORE_DATA ) {

				delete[] data;
				bufLen += 1024;
				data = new char[bufLen];
				memset(data,0,bufLen);

				cbData = bufLen-1;
				dwRet = RegQueryValueEx( hK, key.c_str(), NULL, NULL, (LPBYTE) data, &cbData );
			}

			RegCloseKey(hK);
			string rc(data);

			delete[] data;

			if(dwRet != ERROR_SUCCESS) {

				return def;

			}

			return rc;

		}

	};


	void Config::setName(const char *name) {
		if(confname) {
			throw runtime_error(string{"Configuration already set as '"} + confname.c_str() + "'");
		}
		confname = name;
	}

	bool Config::hasGroup(const std::string &group) {
		// FIXME: Implement.
		return true;
	}

	int32_t Config::get(const std::string &group, const std::string &name, const int32_t def) {
		return Controller().get(group,name,def);
	}

	int64_t Config::get(const std::string &group, const std::string &name, const int64_t def) {
		return Controller().get(group,name,def);
	}

	uint32_t Config::get(const std::string &group, const std::string &name, const uint32_t def) {
		return Controller().get(group,name,def);
	}

	uint64_t Config::get(const std::string &group, const std::string &name, const uint64_t def) {
		// FIXME: Implement.
		return def;
	}

	float Config::get(const std::string &group, const std::string &name, const float def) {
		// FIXME: Implement.
		return def;
	}

	double Config::get(const std::string &group, const std::string &name, const double def) {
		// FIXME: Implement.
		return def;
	}

	std::string Config::get(const std::string &group, const std::string &name, const char *def) {
		return Controller().get(group,name,def);
	}

	std::string Config::get(const std::string &group, const std::string &name, const std::string &def) {
		return Controller().get(group,name,def.c_str());
	}

	bool Config::get(const std::string &group, const std::string &name, const bool def) {
		return Controller().get(group,name,(DWORD) def) != 0;
	}

 }



