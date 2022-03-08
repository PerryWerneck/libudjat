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
 #include "../../private.h"
 #include <udjat/win32/exception.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <fcntl.h>

 namespace Udjat {

	HMODULE Module::Controller::open(const char *name, bool required) {

		Config::Value<string> configured("modules",name,(string{"udjat-module-"} + name).c_str());
		Application::LibDir libdir;

		string paths[] = {
			Config::Value<string>("modules","primary-path",Application::LibDir("modules").c_str()),
#if defined(__x86_64__)
			// 64 bit detected
			Config::Value<string>(
				"modules",
				"secondary-path",
				"c:\\msys64\\mingw64\\lib\\udjat-modules\\" PACKAGE_VERSION "\\"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"c:\\msys64\\mingw64\\lib\\udjat-modules\\"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"/mingw64/lib/udjat-modules/" PACKAGE_VERSION "/"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"/mingw64/lib/udjat-modules/"
			),
#elif  defined(__i386__)
			// 32 bit detected
			Config::Value<string>(
				"modules",
				"secondary-path",
				"c:\\msys64\\mingw32\\lib\\udjat-modules\\" PACKAGE_VERSION "\\"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"c:\\msys64\\mingw32\\lib\\udjat-modules\\"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"/mingw32/lib/udjat-modules/" PACKAGE_VERSION "/"
			),
			Config::Value<string>(
				"modules",
				"secondary-path",
				"/mingw32/lib/udjat-modules/"
			),
#endif
		};

		for(size_t ix = 0; ix < (sizeof(paths)/sizeof(paths[0]));ix++) {

			string filename = paths[ix] + configured + ".dll";

			if(access(filename.c_str(),R_OK) == 0) {

				for(auto module : modules) {

					if(!strcasecmp(module->filename().c_str(),filename.c_str())) {
#ifdef DEBUG
						cout << "module\tModule '" << module->name << "' is already loaded" << endl;
#endif // DEBUG
						return (HMODULE) 0;
					}

				}

				// https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya
				HMODULE handle = LoadLibrary(filename.c_str());
				if(handle) {
					return handle;
				}

				cerr << "modules\t" << filename << " " << Win32::Exception::format(GetLastError()) << endl;

			}
#ifdef DEBUG
			else {
				cout << "modules\tNo module in " << filename << endl;
			}
#endif // DEBUG
		}

		if(required) {
			throw runtime_error(string{"Cant load module '"} + name + "'");
		}

		return (HMODULE) 0;

	}

	void Module::Controller::close(HMODULE module) {
		FreeLibrary(module);
	}

	Module * Module::Controller::init(HMODULE handle) {

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wcast-function-type"
		Module * (*init)(void) = (Module * (*)(void)) GetProcAddress(handle,"udjat_module_init");
		#pragma GCC diagnostic pop
		if(!init) {
			throw Win32::Exception("Cant get module init method");
		}

		Module * module = init();
		if(!module) {
			throw runtime_error("Can't initialize module");
		}

		module->handle = handle;

		return module;

	}
 }

