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
 #include <iostream>
 #include <fcntl.h>

 namespace Udjat {

	void Module::Controller::init(const std::string &filename, const XML::Node &node) {

		Logger::String{"Loading '",filename,"'"}.trace("module");

		// https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya
		HMODULE handle = LoadLibrary(filename.c_str());

		if(!handle) {
			throw Win32::Exception();
		}

		try {

			auto module = init(handle,node);
			if(!module) {
				throw runtime_error("Module initialization has failed");
			}

		} catch(...) {

			CloseHandle(handle);
			throw;
		}


	}

	Module * Module::Controller::init(HMODULE handle, const XML::Node &node) {

		Module * module = nullptr;

		//
		// First try from xml
		//
		Module * (*init_from_xml)(const XML::Node &node)
				= (Module * (*)(const XML::Node &node)) getSymbol(handle,"udjat_module_init_from_xml",false);

		if(init_from_xml) {

			module = init_from_xml(node);
			if(!module) {
				throw runtime_error("Can't initialize module from XML");
			}

			module->handle = handle;
			if(module->gettext_package() && *module->gettext_package()) {
				Application::set_gettext_package(module->gettext_package());
			}

		} else {

			//
			// Not found, try non xml version.
			//
			module = init(handle);

		}

		module->keep_loaded = Object::getAttribute(node, "modules", "keep-loaded", module->keep_loaded);

		return module;
	}

	Module * Module::Controller::init(HMODULE handle) {

		Module * (*init)(void) = (Module * (*)(void)) getSymbol(handle,"udjat_module_init");

		Module * module = init();
		if(!module) {
			throw runtime_error("Can't initialize module");
		}

		module->handle = handle;
		if(module->gettext_package() && *module->gettext_package()) {
			Application::set_gettext_package(module->gettext_package());
		}

		return module;

	}

 }

