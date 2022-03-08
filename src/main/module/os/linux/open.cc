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
 #include <dlfcn.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <unistd.h>

 namespace Udjat {

	void * Module::Controller::open(const char *name, bool required) {

		Config::Value<string> configured("modules",name,(string{"udjat-module-"} + name).c_str());

		string paths[] = {
			Config::Value<string>("modules","primary-path",Application::LibDir("modules/" PACKAGE_VERSION).c_str()),
			Config::Value<string>("modules","secondary-path",Application::LibDir("modules").c_str()),
#ifdef LIBDIR
			Config::Value<string>("modules","common-path",STRINGIZE_VALUE_OF(LIBDIR) "/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "-modules/" PACKAGE_VERSION "/").c_str(),
#endif //LIBDIR
		};

		for(size_t ix = 0; ix < (sizeof(paths)/sizeof(paths[0]));ix++) {

			string filename = paths[ix] + configured + ".so";

			if(access(filename.c_str(),R_OK) == 0) {

				for(auto module : modules) {

					if(!strcasecmp(module->filename().c_str(),filename.c_str())) {
#ifdef DEBUG
						cout << "module\tModule '" << module->name << "' is already loaded" << endl;
#endif // DEBUG
						return NULL;
					}

				}

				dlerror();
				void * handle = dlopen(filename.c_str(),RTLD_NOW|RTLD_LOCAL);
				if(handle) {
					return handle;
				}
				cerr << "modules\t" << filename << " " << dlerror() << endl;
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

		return NULL;

	}

	void Module::Controller::close(void *module) {
		dlclose(module);
	}

	Module * Module::Controller::init(void * handle) {

		Module * (*init)(void) = (Module * (*)(void)) dlsym(handle,"udjat_module_init");
		auto err = dlerror();
		if(err)
			throw runtime_error(err);

		Module * module = init();
		if(!module) {
			throw runtime_error("Can't initialize module");
		}

		module->handle = handle;

		return module;
	}


 }

