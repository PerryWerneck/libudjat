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
 #include <iostream>
 #include <fcntl.h>

 namespace Udjat {

	bool Module::Controller::load(const std::string &filename, const Udjat::Properties &props) {

#ifdef UDJAT_STATIC
		throw logic_error("Cant use dynamic modules on static libudjat");
#else
		if(find_by_filename(filename.c_str()) || find_by_name(filename.c_str())) {
			Logger::String{"Module '",filename.c_str(),"' is already loaded"}.trace();
			return true;
		}

		Logger::String{"Loading '",filename.c_str(),"'"}.trace();

		// Load module.
		// https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya
		HMODULE handle = LoadLibrary(filename.c_str());
		if(!handle) {
			throw Win32::Exception();
		}

		try {

			auto init = getfunc<Module *,const Udjat::Properties &>(handle,"udjat_module_init",false);

			if(!init) {
				throw runtime_error(String{filename.c_str()," is not a valid module"});
			}

			auto module = init(props);
			if(!module) {
				throw runtime_error(String{"Initialization of ",filename.c_str()," has failed"});
			}

			module->handle = handle;
			module->keep_loaded = props.get("keep-loaded",false);
			module->keep_active = props.get("keep-active",false);

			if(props.get("verbose",true) && module->info.description && *module->info.description) {
				Logger::String{module->info.description," version ",module->info.version," initialized"}.info(module->name());
			}

			if(module->info.gettext_package && *module->info.gettext_package) {
				Application::set_gettext_package(module->info.gettext_package);
			}

		} catch(...) {

			CloseHandle(handle);
			throw;

		}

		return false;
#endif // UDJAT_STATIC
	}

 }

