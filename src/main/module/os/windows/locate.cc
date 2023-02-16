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
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <udjat/win32/path.h>
 #include <fcntl.h>

 namespace Udjat {

	std::string Module::Controller::locate(const char *name) noexcept {

		string paths[] = {

			Config::Value<string>("modules","primary-path",Application::LibDir("modules",false).c_str()),

#if defined(LIBDIR)
			Config::Value<string>(
				"modules",
				"versioned-path",
				STRINGIZE_VALUE_OF(LIBDIR) "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/" PACKAGE_VERSION "/modules/"
			),
			Config::Value<string>(
				"modules",
				"unversioned-path",
				STRINGIZE_VALUE_OF(LIBDIR) "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/modules/"
			),
#elif defined(__x86_64__)
			Config::Value<string>(
				"modules",
				"versioned-path",
				"c:\\msys64\\mingw64\\lib\\" STRINGIZE_VALUE_OF(PRODUCT_NAME) "\\" PACKAGE_VERSION "\\modules\\"
			),
			Config::Value<string>(
				"modules",
				"unversioned-path",
				"c:\\msys64\\mingw64\\lib\\" STRINGIZE_VALUE_OF(PRODUCT_NAME) "\\modules\\"
			),
#elif  defined(__i386__)
			Config::Value<string>(
				"modules",
				"versioned-path",
				"c:\\msys64\\mingw32\\lib\\" STRINGIZE_VALUE_OF(PRODUCT_NAME) "\\" PACKAGE_VERSION "\\modules\\"
			),
			Config::Value<string>(
				"modules",
				"unversioned-path",
				"c:\\msys64\\mingw32\\lib\\" STRINGIZE_VALUE_OF(PRODUCT_NAME) "\\modules\\"
			),
#endif // LIBDIR
		};

		if(name && *name) {

			for(const string &path : paths) {

				if(path.empty()) {
					continue;
				}

				Win32::Path filename{(path + STRINGIZE_VALUE_OF(PRODUCT_NAME) "-module-" + name + ".dll").c_str()};

				if(access(filename.c_str(),R_OK) == 0) {
					debug("Found '",filename.c_str(),"' = ",access(filename.c_str(),R_OK));
					return filename;
				}
#ifdef DEBUG
				else {
					debug("Searching '",filename.c_str(),"' = ",access(filename.c_str(),R_OK));
				}
#endif // DEBUG

			}

			// Try with the alternative name.
			Config::Value<string> altname{"modules",name,""};

			if(!altname.empty()) {

				for(const string &path : paths) {

					string filename = path + altname.c_str() + ".so";

					debug("Searching '",filename,"' = ",access(filename.c_str(),R_OK));

					if(access(filename.c_str(),R_OK) == 0) {
						return filename;
					}

				}
			}


		}

		return "";

	}


 }

