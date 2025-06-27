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
		Module::Controller::getInstance().clear();
	}

	void Module::Controller::clear() {

		debug("Unloading ",objects.size()," modules");
#ifdef DEBUG
		{
			string names;
			for(auto module : objects) {
				if(!names.empty()) {
					names += " ";
				}
				names += module->name;
			}
			debug("----> Modules to remove: ",names.c_str());
		}
#endif

		while(objects.size()) {

			Module * module;
			{
				lock_guard<mutex> lock(guard);
				if(objects.empty()) {
					break;
				}
				module = objects.back();
				objects.pop_back();
			}

			// Save module name.
			string name{module->name};
			string description{module->description()};

			auto handle = module->handle;
			auto keep_loaded = module->keep_loaded;

			Logger::String{(keep_loaded ? "Deactivating" : "Unloading")," '",description,"'"}.trace(name);

			try {

				// First delete module

				debug("Deleting module '",name,"'");
				delete module;
				debug("Module '",name,"' deleted");

				if(handle) {

					debug("Deinitializing module '",name,"'");
					if(!deinit(handle)) {
						clog << name << "\tKeeping module loaded by deinit() request" << endl;
						continue;
					}

					if(keep_loaded) {
						clog << name << "\tKeeping module loaded by configuration request" << endl;
					} else {
						debug("Unloading module '",name,"'");
						unload(handle,name,description);
						debug("Module '",name,"' unloaded");
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

