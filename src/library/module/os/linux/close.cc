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

	bool Module::Controller::deinit(void *handle) {
		bool (*udjat_module_deinit)(void) = (bool (*)(void)) ::dlsym(handle,"udjat_module_deinit");
		auto err = dlerror();
		if(!err) {
			debug("Calling udjat_module_deinit");
			bool rc = udjat_module_deinit();
			debug("(udjat_module_deinit returns ",rc);
			return rc;
		}
		debug("No udjat_module_deinit method, just returning true");
		return true;
	}

	void Module::Controller::unload(void *handle, const string &name, const string &description) const {

		if(dlclose(handle)) {
			Logger::String{"Error '",dlerror(), "' closing module '",name,"'"}.error("modules");
		} else {
			Logger::String{"Module '",name,"' (",description,") was unloaded"}.trace("modules");
		}

	}


 }

