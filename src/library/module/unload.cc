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
#include <udjat/tools/threadpool.h>
#include <udjat/tools/logger.h>

#ifdef _WIN32
	#include <udjat/win32/exception.h>
#else
	#include <dlfcn.h>
#endif // _WIN32

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::unload() {
		Module::Controller::getInstance().unload();
	}

	void Module::Controller::unload() {

		debug("Unloading ",modules.size()," modules");

#ifdef DEBUG
		{
			string names;
			for(auto module : modules) {
				if(!names.empty()) {
					names += " ";
				}
				names += module->module_name;
			}
			debug("----> Modules to remove: ",names.c_str());
		}
#endif

		while(modules.size()) {

			auto module = modules.back();
			modules.remove(module);

			// Save module name.
			string name{module->module_name};
			string description{module->description()};

			auto handle = module->handle;
			auto keep_loaded = module->keep_loaded;
			auto keep_active = module->keep_active;

			Logger::String{(keep_loaded ? "Deactivating" : "Unloading")," '",description,"'"}.trace(name);

			try {

				// First delete module

				if(!keep_active) {

					Logger::String{"Deactivating module '",description,"'"}.write(Logger::Debug,name.c_str());
					delete module;

					if(handle && !keep_loaded) {

						if(!deinit(handle)) {
							Logger::String{"Keeping module loaded by deinit() request"}.trace(name.c_str());
							continue;
						}

						Logger::String{"Unloading module '",description,"'"}.write(Logger::Debug,name.c_str());
						unload(handle,name,description);

					}

				}


			} catch(const exception &e) {
				cerr << name << "\tError '" << e.what() << "' deinitializing module" << endl;
			} catch(...) {
				cerr << name << "\tUnexpected error deinitializing module" << endl;
			}

		}
		debug("Module unloading complete");

	}

}

