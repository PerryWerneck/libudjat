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

	/*
	bool Module::Controller::load(const pugi::xml_node &node) {

		string filename = locate(node.attribute("name").as_string());

		if(!filename.empty()) {

			for(auto module : modules) {

				// Check if the module is already loaded.
				if(!strcasecmp(module->filename().c_str(),filename.c_str())) {
#ifdef DEBUG
					debug("module '",filename,"' is already loaded");
#endif // DEBUG
					return true;
				}

			}

			init(filename, node);

		}

		// Not found.
		if(node.attribute("required").as_bool(true)) {
			throw runtime_error(string{"Cant load module '"}+node.attribute("name").as_string() + "'");
		}

		return false;
	}
	*/

 }

