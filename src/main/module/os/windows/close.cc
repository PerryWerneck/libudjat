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
 #include <udjat/tools/logger.h>
 #include <fcntl.h>

 namespace Udjat {

	void Module::Controller::close(HMODULE module) {
		FreeLibrary(module);
	}

	bool Module::Controller::deinit(HMODULE handle) {

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wcast-function-type"
		bool (*udjat_module_deinit)(void) = (bool (*)(void)) GetProcAddress(handle,"udjat_module_deinit");
		#pragma GCC diagnostic pop

		if(udjat_module_deinit) {
			trace("Calling udjat_module_deinit");
			bool rc = udjat_module_deinit();
			trace("(udjat_module_deinit returns ",rc);
			return rc;
		}
		trace("No udjat_module_deinit method, just returning true");
		return true;
	}

	void Module::Controller::unload(HMODULE handle, const string &name, const string &description) const {

		// https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-freelibrary
		if(FreeLibrary(handle) == 0) {
			cerr << "modules\tError '" << GetLastError() << "' freeing module '" << name << "'" << endl;
		} else {
			cout << "modules\tModule '" << name << "' (" << description << ") was unloaded" << endl;
		}


	}

 }

