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

 #ifndef _GNU_SOURCE
	#define _GNU_SOURCE
 #endif // !_GNU_SOURCE

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/application.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <system_error>
 #include <libintl.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	void Application::set_gettext_package(const char *gettext_package) {

		bindtextdomain(gettext_package, STRINGIZE_VALUE_OF(LOCALEDIR));
		bind_textdomain_codeset(gettext_package, "UTF-8");

#ifdef DEBUG
		cout << "locale\tInitialized using " << STRINGIZE_VALUE_OF(LOCALEDIR) << "/" << gettext_package << endl;
#endif // DEBUG

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


	Application::Name::Name(bool with_path) : string{with_path ? program_invocation_name : program_invocation_short_name} {
	}

	const Application::Name & Application::Name::getInstance() {
		static const Application::Name instance;
		return instance;
	}

	Application::DataDir::DataDir() : File::Path{STRINGIZE_VALUE_OF(DATADIR) "/"} {
		append(program_invocation_short_name);
		append("/");
	}

	Application::DataDir::DataDir(const char *subdir) : DataDir() {
		append(subdir);
		if(mkdir(c_str(),0755)) {
			if(errno != EEXIST) {
				throw system_error(errno,system_category(),c_str());
			}
		}
		append("/");
	}

	Application::DataFile::DataFile(const char *name) {
		if(name[0] == '/' || (name[0] == '.' && name[1] == '/')) {
			assign(name);
		} else {
			assign(DataDir());
			append(name);
		}
	}

	Application::LibDir::LibDir() : string{STRINGIZE_VALUE_OF(LIBDIR) "/"} {
	}

	Application::LibDir::LibDir(const char *subdir) : LibDir() {
		append(program_invocation_short_name);
		append("-");
		append(subdir);
		append("/");
	}

	void Application::LibDir::reset(const char *application_name, const char *subdir) {
		assign(STRINGIZE_VALUE_OF(LIBDIR) "/");
		append(application_name);
		append("-");
		append(subdir);
		append("/");
	}

	Application::LibDir::operator bool() const noexcept {
		return (access(c_str(), R_OK) == 0);
	}

	Application::SysConfigDir::SysConfigDir() : string{"/etc/"} {
	}

	Application::SysConfigDir::SysConfigDir(const char *subdir) : SysConfigDir() {
		append(program_invocation_short_name);
		append("/");
		append(subdir);
		append("/");
	}

	Application::CacheDir::CacheDir() : string{"/var/cache/"} {

		append(program_invocation_short_name);
		append("/");

		if(mkdir(c_str(), 0700) == 0)
			return;

		if(access(c_str(),W_OK) == 0)
			return;

		const char *homedir = getenv("HOME");
		if(!homedir)
			homedir = "~";

		assign(homedir);
		append("/.cache/");

		append(program_invocation_short_name);
		append("/");

		if(mkdir(c_str(), 0700) == 0)
			return;

		if(access(c_str(),W_OK) == 0)
			return;

		throw system_error(EPERM,system_category(),c_str());

	}

	Application::CacheDir::CacheDir(const char *subdir) : CacheDir() {

		append(subdir);
		append("/");

		if(mkdir(c_str(), 0700) == 0)
			return;

		if(access(c_str(),W_OK) == 0)
			return;

		throw system_error(EPERM,system_category(),c_str());

	}

 }
