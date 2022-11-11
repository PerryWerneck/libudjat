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
 #include <fcntl.h>

 namespace Udjat {

	std::string Module::Controller::locate(const char *name) noexcept {

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

		if(name && *name) {

			for(const string &path : paths) {

				string filename = path + STRINGIZE_VALUE_OF(PRODUCT_NAME) "-module-" + name + ".dll";
				debug("Searching '",filename,"' = ",access(filename.c_str(),R_OK));

				if(access(filename.c_str(),R_OK) == 0) {
					return filename;
				}

			}

		}

		return "";

	}


 }

