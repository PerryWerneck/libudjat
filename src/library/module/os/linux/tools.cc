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
 #include <unistd.h>

 namespace Udjat {

	Module * Module::factory(const char *filename) {

		dlerror();
		void * handle = dlopen(filename,RTLD_NOW|RTLD_LOCAL);
		if(!handle) {
			throw runtime_error(dlerror());
		}

		try {

			return Controller::init(handle);

		} catch(...) {

			dlclose(handle);
			throw;

		}

	}

	Module * Module::Controller::find_by_filename(const char *path) {

		for(auto module : objects) {

			// Check if the module is already loaded.
			if(!strcasecmp(module->filename().c_str(),path)) {
				return module;
			}

		}

		return nullptr;

	}
	
	void * Module::Controller::getSymbol(void *handle, const char *name	, bool required) {

		void * symbol = ::dlsym(handle,name);

		if(required) {
			auto err = dlerror();
			if(err)
				throw runtime_error(err);
		}

		return symbol;
	}
	
	std::string Module::filename() const {
		Dl_info info;
		memset(&info,0,sizeof(info));
		if(dladdr(&this->_info, &info) != 0 && info.dli_fname && info.dli_fname[0]) {
			return info.dli_fname;
		}
		return name;
	}

 }

