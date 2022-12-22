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
#include <udjat/tools/logger.h>
#include <udjat/tools/xml.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	bool Module::Controller::load(const pugi::xml_node &node) {

		static const char * attributes[] = {
			"name",
			"altname",
			"fallback-to"
		};

		for(const char *attribute : attributes) {

			string filename = locate(node.attribute(attribute).as_string());

			if(!filename.empty()) {

				for(auto module : modules) {

					// Check if the module is already loaded.
					if(!strcasecmp(module->filename().c_str(),filename.c_str())) {
						debug("module '",filename,"' is already loaded");
						return true;
					}

				}

				init(filename, node);
				return false;

			}

		}

		// Not found.
		if(node.attribute("required").as_bool(true)) {
			throw runtime_error(string{"Cant load required module '"} + node.attribute(attributes[0]).as_string() + "'");
		}

		return false;
	}

	bool Module::preload(const char *pathname) noexcept {

		bool rc = true;

		// Preload modules from config
		/*
		{
			Config::Value<std::vector<std::string>> modules{"modules","load-at-startup",""};

			debug("load-at-startup size=",modules.size());
			if(modules.size()) {
				Logger::String("Preloading ",modules.size()," module(s) from configuration file").trace("module");
			}
		}
		*/

		// Preload modules from XML
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

	void Module::load(const char *path, bool required) {

		File::Path{path}.for_each([required](const File::Path &path){

#ifdef _WIN32
			if(!path.match("*.dll")) {
				Logger::String{"Ignoring file '",path,"'"}.trace("module");
				return true;
			}
#else
			if(!path.match("*.so")) {
				Logger::String{"Ignoring file '",path,"'"}.trace("module");
				return true;
			}
#endif // _WIN32

			cerr << "Unable to load '" << path << "': " << strerror(ENOTSUP) << endl;

			return true;
		},true);

	}

}

