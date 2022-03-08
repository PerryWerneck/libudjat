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
 #include "../../private.h"
 #include <udjat/win32/exception.h>

 namespace Udjat {

	HMODULE Module::Controller::open(const char *filename, bool required) {

		// https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya
		HMODULE handle = LoadLibrary(filename);
		if(!handle) {
			string message = Win32::Exception::format(GetLastError());

			if(required) {
				throw runtime_error(string{"Cant load '"} + filename + "' - " + message.c_str());
			}

			clog << "module\tCant load '" << filename << "': " << message << endl;

				return nullptr;
		}

		return handle;

	}

	Module * Module::Controller::init(HMODULE handle) {

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wcast-function-type"
		Module * (*init)(void) = (Module * (*)(void)) GetProcAddress(handle,"udjat_module_init");
		#pragma GCC diagnostic pop
		if(!init) {
			throw Win32::Exception("Cant get module init method");
		}

		Module * module = init();
		if(!module) {
			throw runtime_error("Can't initialize module");
		}

		module->handle = handle;

		return module;

	}
 }
