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
 #include <udjat/tools/configuration.h>
 #include <udjat/module.h>
 #include <errno.h>
 #include <stdexcept>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <udjat/win32/registry.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/charset.h>
 #include <cstring>
 #include <iostream>
 #include <private/win32.h>
 #include "private.h"

 #ifdef HAVE_LIBINTL
	#include <libintl.h>
 #endif // HAVE_LIBINTL

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

			WSADATA WSAData;
			WSAStartup(0x101, &WSAData);

#ifdef GETTEXT_PACKAGE
			set_gettext_package(GETTEXT_PACKAGE);
			setlocale( LC_ALL, "" );
#endif // GETTEXT_PACKAGE

			// https://github.com/alf-p-steinbach/Windows-GUI-stuff-in-C-tutorial-/blob/master/docs/part-04.md
			SetConsoleOutputCP(CP_UTF8);
			SetConsoleCP(CP_UTF8);

			if(Config::Value<bool>("application","virtual-terminal-processing",true)) {
				// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
				HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
				if(hOut != INVALID_HANDLE_VALUE) {
					DWORD dwMode = 0;
					if(GetConsoleMode(hOut, &dwMode)) {
						dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
						SetConsoleMode(hOut,dwMode);
					}
				}
			}

			if(!Module::preload()) {
				throw runtime_error("Module preload has failed");
			}

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

	Application::DataDir::DataDir(const char *subdir) : File::Path{Application::Path{subdir}} {
	}

	Application::SystemDataDir::SystemDataDir() {

		try {

			assign(Win32::Registry().get("systemdatadir",""));
			if(!empty()) {
				mkdir();
				return;
			}

		} catch(...) {
			// Ignore errors.
		}

		assign(Win32::KnownFolder(FOLDERID_ProgramData));

		append(Application::Name());
		mkdir();
		append("\\");

	}

	Application::UserDataDir::UserDataDir(const char *subdir) : File::Path() {

		try {

			assign(Win32::Registry().get("userdatadir",""));
			if(!empty()) {
				mkdir();
				return;
			}

		} catch(...) {
			// Ignore errors.
		}

#ifdef FOLDERID_AppDataDocuments
		assign(Win32::KnownFolder(FOLDERID_AppDataDocuments));
#else
		assign(Win32::KnownFolder(FOLDERID_Documents));
#endif // FOLDERID_AppDataDocuments

		append(Application::Name());

		if(subdir && *subdir) {
			append("\\");
			append(subdir);
		}

		mkdir();
		append("\\");

	}

	Application::LibDir::LibDir(const char *subdir) : File::Path{Application::Path{subdir}} {
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

	Application::SysConfigDir::SysConfigDir(const char *subdir) : File::Path{Application::Path{subdir}} {
	}

	Application::LogDir::LogDir(const char *subdir) : File::Path{PathFactory(FOLDERID_ProgramData,"log")} {
		if(subdir && *subdir) {
			append(subdir);
			mkdir();
			append("\\");
		}
	}

	Application::CacheDir::CacheDir(const char *subdir) : File::Path{PathFactory(FOLDERID_ProgramData,"cache")} {
		if(subdir && *subdir) {
			append(subdir);
			mkdir();
			append("\\");
		}
	}

 }
