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
 #include <fcntl.h>

 namespace Udjat {

	Module * Module::Controller::find_by_filename(const char *filename) {

		char path[MAX_PATH+1];
		memset(path,0,MAX_PATH+1);

		if(!GetFullPathName(filename,MAX_PATH,path,NULL)) {
			cerr << "module\t" << filename << ": " << Win32::Exception::format(GetLastError()) << endl;
			strncpy(path,filename,MAX_PATH);
		}

		for(auto module : objects) {

			// Check if the module is already loaded.
			if(!strcasecmp(module->filename().c_str(),path)) {
				return module;
			}

		}

		return nullptr;

	}

	void Module::trace_paths(const char *) const noexcept {
	}

	std::string Module::filename() const {
		TCHAR path[MAX_PATH];
		if(GetModuleFileName(this->handle, path, MAX_PATH) ) {
			return (const char *) path;
		}
		return name;
	}

	void * Module::Controller::getSymbol(HMODULE hModule, const char *name, bool required) {

		void * symbol = (void *) GetProcAddress(hModule,name);

		if(required && !symbol) {
			throw Win32::Exception(string{"Can't find symbol '"} + name + "'");
		}

		return symbol;
	}


 }

