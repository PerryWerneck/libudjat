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
 #include <udjat/tools/logger.h>
 #include <udjat/module.h>
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

#ifdef HAVE_LIBINTL

		bindtextdomain(gettext_package, STRINGIZE_VALUE_OF(LOCALEDIR));
		bind_textdomain_codeset(gettext_package, "UTF-8");
		debug("Locale set to ",STRINGIZE_VALUE_OF(LOCALEDIR),"/",gettext_package);

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

		if(!Module::preload()) {
			throw runtime_error("Module preload has failed, aborting service");
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
		mkdir();
		append("/");
	}

	Application::LogDir::LogDir() : File::Path{"/var/log/"} {
		append(program_invocation_short_name);
		mkdir();
		append("/");
	}

	Application::LogDir::LogDir(const char *subdir) : LogDir() {
		append(subdir);
		mkdir();
		append("/");
	}

	Application::SystemDataDir::SystemDataDir() : File::Path{"/usr/share/"} {
		append(program_invocation_short_name);
		mkdir();
		append("/");
	}

	Application::DataDir::DataDir(const char *subdir) : DataDir() {
		append(subdir);
		mkdir();
		append("/");
	}

	Application::LibDir::LibDir() : string{STRINGIZE_VALUE_OF(LIBDIR) "/"} {
	}

	Application::LibDir::LibDir(const char *subdir) : LibDir() {
		append(program_invocation_short_name);
		append("/");
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

 }
