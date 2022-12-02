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
		while(modules.size()) {

			Module * module;
			{
				lock_guard<recursive_mutex> lock(guard);
				if(modules.empty()) {
					break;
				}
				module = modules.back();
				modules.pop_back();
			}

			// Save module name.
			string name{module->name};
			string description{module->description()};

			auto handle = module->handle;
			auto keep_loaded = module->keep_loaded;

#ifdef DEBUG
			cout << name << "\tkeep-loaded=" << (keep_loaded ? "ON" : "OFF") << endl;
#endif // DEBUG
			cout << name << "\t" << (keep_loaded ? "Deactivating" : "Unloading") << " '" << description << "'" << endl;

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
					debug("Module '",name,"' deinitialized");

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

