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
 #include <private/module.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <fcntl.h>

 namespace Udjat {

	bool Module::Controller::load(const pugi::xml_node &node) {

		const char *name = node.attribute("name").as_string();
		if(!*name) {
			throw runtime_error("Missing required attribute 'name'");
		}

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
			Config::Value<string>(
				"modules",
				"sysroot",
				"c:\\msys64\\mingw64\\"
			) + "lib\\udjat-modules\\" PACKAGE_VERSION "\\",
			Config::Value<string>(
				"modules",
				"sysroot",
				"c:\\msys64\\mingw64\\"
			) + "lib\\udjat-modules\\",
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
						return true;
					}

				}

				// https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya
				HMODULE handle = LoadLibrary(filename.c_str());
				if(handle) {

					cout << name << "\tLoading module from " << filename << endl;

					try {

						auto module = init(handle,node);
						if(!module) {
							throw runtime_error("Module initialization has failed");
						}

					} catch(...) {

						CloseHandle(handle);
						throw;
					}

					return false;

				}

				cerr << "modules\t" << filename << " " << Win32::Exception::format(GetLastError()) << endl;

			}
#ifdef DEBUG
			else {
				debug("No module in '",filename,"'");
			}
#endif // DEBUG
		}


		// Not found.
		if(node.attribute("required").as_bool(true)) {
			throw runtime_error(string{"Cant load module '"} + name + "'");
		}

		return false;

	}

 }

