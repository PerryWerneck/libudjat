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

#define GNU_SOURCE
#include <config.h>
#include <private/module.h>
#include <sys/types.h>
#include <dirent.h>
#include <udjat/tools/file.h>
#include <udjat/tools/application.h>
#include <udjat/tools/configuration.h>
#include <udjat/tools/object.h>
#include <udjat/tools/xml.h>

#ifdef _WIN32
	#define MODULE_EXT ".dll"
	#include <udjat/win32/exception.h>
#else
	#include <dlfcn.h>
	#include <unistd.h>
	#define MODULE_EXT ".so"
#endif // _WIN32

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	bool Module::preload(const char *pathname) noexcept {

		bool rc = true;

		/*
		// Preload from configuration file.
		{
			Config::Value<vector<string>> modules("modules","load-at-startup","");

			if(!modules.empty()) {
				cout << "modules\tPreloading " << modules.size() << " module(s) from configuration file" << endl;

				Config::Value<bool> required("modules","required",false);
				for(string &module : modules) {

					try {


						load(module.c_str(),required);

					} catch(const std::exception &e) {

						cerr << "modules\tCant load '" << module << "': " << e.what() << endl;
						rc = false;

					}

				}
			}

		}
		*/

		if(pathname && *pathname && Config::Value<bool>("modules","preload-from-xml",true)) {

			// Preload from path.
			cout << "modules\tPreloading from " << pathname << endl;
			Udjat::for_each(pathname, [&rc](const char UDJAT_UNUSED(*filename), const pugi::xml_document &doc){
				for(pugi::xml_node node = doc.document_element().child("module"); node; node = node.next_sibling("module")) {
					if(node.attribute("preload").as_bool(false)) {

						try {

							Module::load(node);

						} catch(const std::exception &e) {

							cerr << "modules\t" << e.what() << endl;
							rc = false;

						}

					}
				}
			});

		}

		return rc;
	}

	void Module::load(const pugi::xml_node &node) {
		Controller::getInstance().load(node);
	}

	void Module::load(const char *name, bool required) {
		pugi::xml_node node;
		node.append_attribute("required").set_value(required);
		load(node);
	}

	/*
	void Module::Controller::load(const pugi::xml_node &node) {

		const char * name = node.attribute("name").as_string();

		if(!(name && *name)) {
			throw runtime_error("Required attribute 'name' is missing");
		}

		// Check if the module is already loaded.
		if(find(name)) {
#ifdef DEBUG
			cout << "module\t**** The module '" << name << "' was already loaded" << endl;
#endif // DEBUG
			return;
		}

#ifdef DEBUG
			cout << "module\t**** Opening module '" << name << "'" << endl;
#endif // DEBUG

		// Open module.
		auto handle = open(name,Object::getAttribute(node,"modules","required",true));

		if(handle) {

			try {

				auto module = init(handle,node);
				module->handle = handle;

			} catch(...) {

				close(handle);
				throw;

			}

		}

	}

	void Module::Controller::load(const char *name, bool required) {

		// Check if the module is already loaded.
		if(find(name)) {
#ifdef DEBUG
			cout << "module\t**** The module '" << name << "' was already loaded" << endl;
#endif // DEBUG
			return;
		}

		auto handle = open(name,required);

		if(handle) {

			try {

				auto module = init(handle);
				module->handle = handle;

			} catch(...) {

				close(handle);
				throw;

			}

		}

	}
	*/

}

