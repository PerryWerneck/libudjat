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
 #include <udjat/tools/logger.h>
 #include <errno.h>
 #include <stdexcept>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <udjat/win32/registry.h>
 #include <udjat/win32/exception.h>
 #include <cstring>
 #include <iostream>
 #include <libintl.h>

 using namespace std;

 namespace Udjat {

	void Application::set_gettext_package(const char *gettext_package) {

#ifdef HAVE_LIBINTL

		Path localedir;
		localedir += "locale";

		if(access(localedir.c_str(),R_OK) == 0) {
			bindtextdomain(gettext_package, localedir.c_str());
			bind_textdomain_codeset(gettext_package, "UTF-8");
		}

#ifdef DEBUG
		cout << "locale\tInitialized using " << localedir << endl;
#endif // DEBUG

#endif // HAVE_LIBINTL

	}

	bool Application::init() {
		static bool initialized = false;

		if(!initialized) {
			initialized = true;
#ifdef GETTEXT_PACKAGE
			set_gettext_package(GETTEXT_PACKAGE);
			setlocale( LC_ALL, "" );
#endif // GETTEXT_PACKAGE
			return true;
		}
		return false;
	}

	Application::Name::Name(bool with_path) {

		char *ptr;
		TCHAR filename[MAX_PATH];

		if(!GetModuleFileName(NULL, filename, MAX_PATH ) ) {
			throw runtime_error("Can't get application filename");
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

	Application::DataDir::DataDir() : File::Path(Application::Path()) {
	}

	Application::DataDir::DataDir(const char *subdir) : DataDir() {
		append(subdir);
		mkdir(c_str());
		append("\\");
	}

	Application::DataFile::DataFile(const char *name) {
		if(name[0] == '/' || (name[0] == '.' && name[1] == '/') || name[0] == '\\' || (name[0] == '.' && name[1] == '\\') || name[1] == ':' ) {
			assign(name);
		} else {
			assign(DataDir());
			append(name);
		}
	}

	Application::InstallLocation::operator bool() const {

		if(empty()) {
			return false;
		}

		trace("InstallLocation='",c_str(),"'");
		trace("ApplicationPath='",Application::Path().c_str(),"'");
		return strcmp(c_str(),Application::Path().c_str()) == 0;

	}

	Application::InstallLocation::InstallLocation() {

		string path{"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"};
		path += Application::Name();

		trace("Searching for 'InstallLocation' in '",path,"'");

		static const DWORD options[] = { KEY_READ|KEY_WOW64_32KEY, KEY_READ|KEY_WOW64_64KEY };

		for(size_t ix = 0; ix < (sizeof(options)/sizeof(options[0])); ix++) {
			HKEY hKey;
			LSTATUS rc =
				RegOpenKeyEx(
					HKEY_LOCAL_MACHINE,
					TEXT(path.c_str()),
					0,
					options[ix],
					&hKey
				);

			if(rc == ERROR_SUCCESS) {
				Win32::Registry registry{hKey};
				if(registry.hasValue("InstallLocation")) {
					assign(registry.get("InstallLocation",""));
					trace("InstallLocation='",c_str(),"'");
				}
			} else if(rc != ERROR_FILE_NOT_FOUND) {
				cerr << "win32\t" << Win32::Exception::format(path.c_str()) << endl;
			}
		}

		trace("No 'InstallLocation' registry key");

	}

	Application::LibDir::LibDir() : string(Application::Path()) {
	}

	Application::LibDir::LibDir(const char *subdir) : Application::LibDir() {
		append(subdir);
		append("\\");
		mkdir(c_str());
	}

	void Application::LibDir::reset(const char *application_name, const char *subdir) {

		if(application_name && *application_name) {

			// Search application install dir.
			try {

				Win32::Registry registry((string{"\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"}+application_name).c_str());
				assign(registry.get("InstallLocation",""));
				if(!empty()) {
					append(subdir);
					append("\\");
					return;
				}

			} catch(const std::exception &e) {

				cerr << "win32\tError '" << e.what() << "' finding installation path of '" << application_name << "'" << endl;

			} catch(...) {

				cerr << "win32\tUnexpected error finding installation path of '" << application_name << "'" << endl;

			}


		}

		assign(Application::Path());
		append(subdir);
		append("\\");
	}

	Application::LibDir::operator bool() const noexcept {
		return (access(c_str(), R_OK) == 0);
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

	Application::CacheDir::CacheDir() {

		try {

			Win32::Registry registry;
			assign(registry.get("cachedir",""));
			if(!empty()) {
				mkdir(c_str());
				return;
			}

		} catch(...) {
			// Ignore errors.
		}

		assign(Application::Path());
		append("\\cache\\");
		mkdir(c_str());
	}

	Application::CacheDir::CacheDir(const char *subdir) : CacheDir() {

		append(subdir);
		append("\\");

	}

 }
