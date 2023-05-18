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
 #include <udjat/module.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>

 namespace Udjat {

	std::vector<std::string> Module::search_path() noexcept {

		return std::vector<string>{
#ifdef MODULES_DIR
			Config::Value<string>("modules","application-path",STRINGIZE_VALUE_OF(MODULES_DIR)).c_str(),
#endif // MODULES_DIR
			Config::Value<string>("modules","path",Application::LibDir(PACKAGE_VERSION "/modules",false).c_str()),
#ifdef LIBDIR
			Config::Value<string>("modules","common-path",STRINGIZE_VALUE_OF(LIBDIR) "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/" PACKAGE_VERSION "/modules/").c_str(),
			Config::Value<string>("modules","compatibility-path",STRINGIZE_VALUE_OF(LIBDIR) "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "-modules/" PACKAGE_VERSION "/").c_str(),
#endif //LIBDIR
		};

	}

	std::string Module::Controller::locate(const char *name) noexcept {

		std::vector<std::string> paths{Module::search_path()};

		if(name && *name) {

			// Try using name
			for(const string &path : paths) {

				if(path.empty()) {
					continue;
				}

				string filename = path + STRINGIZE_VALUE_OF(PRODUCT_NAME) "-module-" + name + LIBEXT;

				debug("Searching '",filename,"' = ",access(filename.c_str(),R_OK));

				if(access(filename.c_str(),R_OK) == 0) {
					return filename;
				}

			}

			// Try with the alternative name.
			Config::Value<string> altname{"modules",name,""};

			if(!altname.empty()) {

				for(const string &path : paths) {

					if(path.empty()) {
						continue;
					}

					string filename = path + altname.c_str() + LIBEXT;

					debug("Searching '",filename,"' = ",access(filename.c_str(),R_OK));

					if(access(filename.c_str(),R_OK) == 0) {
						return filename;
					}

				}
			}

			// Last change, use name without product name.
			for(const string &path : paths) {

				if(path.empty()) {
					continue;
				}

				string filename = path + name + LIBEXT;

				debug("Searching '",filename,"' = ",access(filename.c_str(),R_OK));

				if(access(filename.c_str(),R_OK) == 0) {
					return filename;
				}

			}

		}

		return "";

	}

 }

