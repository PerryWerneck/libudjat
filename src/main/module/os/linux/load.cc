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
 #include <dlfcn.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>

 namespace Udjat {

	bool Module::Controller::load(const pugi::xml_node &node) {

		string paths[] = {
#ifdef MODULES_DIR
			Config::Value<string>("modules","application-path",STRINGIZE_VALUE_OF(MODULES_DIR)).c_str(),
#endif // MODULES_DIR
			Config::Value<string>("modules","primary-path",Application::LibDir("modules/" PACKAGE_VERSION).c_str()),
			Config::Value<string>("modules","secondary-path",Application::LibDir("modules").c_str()),
#ifdef LIBDIR
			Config::Value<string>("modules","common-path",STRINGIZE_VALUE_OF(LIBDIR) "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "-modules/" PACKAGE_VERSION "/").c_str(),
#endif //LIBDIR

		};

		const char *name = node.attribute("name").as_string();
		if(!*name) {
			throw runtime_error("Missing required attribute 'name'");
		}

		for(size_t ix = 0; ix < (sizeof(paths)/sizeof(paths[0]));ix++) {

			string filename = paths[ix] + "udjat-module-" + name + ".so";

			if(access(filename.c_str(),R_OK) == 0) {

				for(auto module : modules) {

					if(!strcasecmp(module->filename().c_str(),filename.c_str())) {
#ifdef DEBUG
						cout << "module\t *** Module '" << module->name << "' is already loaded" << endl;
#endif // DEBUG
						return true;
					}

				}

				dlerror();
				void * handle = dlopen(filename.c_str(),RTLD_NOW|RTLD_LOCAL);
				if(handle) {
					cout << name << "\tLoading module from " << filename << endl;
					try {

						auto module = init(handle,node);
						if(!module) {
							throw runtime_error("Module initialization has failed");
						}

					} catch(...) {

						dlclose(handle);
						throw;

					}

					return false;

				} else {
					cerr << "modules\t" << filename << " " << dlerror() << endl;
				}
			}
#ifdef DEBUG
			else {
				trace("No module in '",filename,"'");
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

