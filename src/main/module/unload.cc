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

#include "private.h"
#include <dlfcn.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Module::unload() {
		Module::Controller::getInstance().unload();
	}

	void Module::Controller::unload() {

		while(modules.size()) {

			Module * module = *modules.begin();

			// Save module name.
			string name(module->name);

			void *handle = module->handle;

			try {

				// First delete module
				delete module;

				if(handle) {
					bool (*deinit)(void) = (bool (*)(void)) dlsym(handle,"udjat_module_deinit");
					auto err = dlerror();
					if(!err) {
						if(!deinit()) {
							cout << name << "\tModule disabled (still open)" << endl;
						} else if(dlclose(handle)) {
							cerr << name << "\tError '" << dlerror() << "' closing module" << endl;
						} else {
							cout << name << "\tModule unloaded" << endl;
						}
					}
				}

			} catch(const exception &e) {
				cerr << name << "\tError '" << e.what() << "' deinitializing module" << endl;
			} catch(...) {
				cerr << name << "\tUnexpected error deinitializing module" << endl;
			}

		}

	}

}

