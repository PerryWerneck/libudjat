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

 #define LOG_DOMAIN "module"

 #include <config.h>
 #include <private/module.h>
 #include <udjat/module.h>
 #include <dlfcn.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	bool Module::Controller::load(const std::string &filename, const Udjat::Properties &props) {

#ifdef LIBUDJAT_STATIC

		throw logic_error("Cant use dynamic modules on static libudjat");

#else
		
		if(find_by_filename(filename.c_str()) || find_by_name(filename.c_str())) {
			Logger::String{"Module '",filename.c_str(),"' is already loaded"}.trace();
			return true;
		}

		Logger::String{"Loading '",filename.c_str(),"'"}.trace();

		// Load module.
		dlerror();
		void * handle = dlopen(filename.c_str(),RTLD_NOW|RTLD_LOCAL);
		if(!handle) {
			throw runtime_error(dlerror());
		}

		try {

			auto init = getfunc<Module *,const Udjat::Properties &>(handle,"udjat_module_init",true);

			if(!init) {
				throw runtime_error(String{filename.c_str()," is not a valid module"});
			}

			debug("Calling init...");
			auto module = init(props);
			debug("Init has returned");
			if(!module) {
				throw runtime_error(String{"Initialization of ",filename.c_str()," has failed"});
			}

			module->handle = handle;
			module->keep_loaded = props.get("keep-loaded",false);
			module->keep_active = props.get("keep-active",false);

			if(props.get("verbose",true) && module->info.description && *module->info.description) {
				Logger::String{module->info.description," version ",module->info.version," initialized (",size()," module(s) loaded)"}.info(module->name());
			}

			if(module->info.gettext_package && *module->info.gettext_package) {
				Application::set_gettext_package(module->info.gettext_package);
			}

		} catch(...) {

			dlclose(handle);
			throw;

		}

		return false;

#endif // LIBUDJAT_STATIC
	}

}

