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

			string name{module->name};
			string description{module->info.description};

			cout << "modules\tUnloading '" << name << "' (" << description << ")" << endl;

			// Save module name.

			auto handle = module->handle;
			auto keep_loaded = module->keep_loaded;

			try {

				// First delete module
#ifdef DEBUG
				cout << "**** Deleting module " << hex << module << dec << endl;
#endif // DEBUG
				delete module;

				if(handle) {

					if(!deinit(handle)) {
						clog << name << "\tKeeping module loaded by deinit() request" << endl;
						continue;
					}

					if(keep_loaded) {
						clog << name << "\tKeeping module loaded by configuration request" << endl;
					} else {
						unload(handle,name,description);
					}

					/*
					if(Config::Value<bool>("modules","keep-loaded",false)) {
					} else  {
						unload(handle,name,description);
					}
					*/

				}

			} catch(const exception &e) {
				cerr << name << "\tError '" << e.what() << "' deinitializing module" << endl;
			} catch(...) {
				cerr << name << "\tUnexpected error deinitializing module" << endl;
			}

		}

	}

}

