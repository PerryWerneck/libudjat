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
 #include <dlfcn.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <unistd.h>

 namespace Udjat {

	Module * Module::Controller::init(void *handle, const pugi::xml_node &node) {

		Module * (*init_from_xml)(const pugi::xml_node &node)
				= (Module * (*)(const pugi::xml_node &node)) getSymbol(handle,"udjat_module_init_from_xml",false);

		if(init_from_xml) {

			Module * module = init_from_xml(node);
			if(!module) {
				throw runtime_error("Can't initialize module");
			}

			module->handle = handle;
			return module;

		}

		return init(handle);

	}

	Module * Module::Controller::init(void * handle) {

		Module * (*init)(void) = (Module * (*)(void)) getSymbol(handle,"udjat_module_init");

		Module * module = init();
		if(!module) {
			throw runtime_error("Can't initialize module");
		}

		module->handle = handle;

		return module;
	}

 }
