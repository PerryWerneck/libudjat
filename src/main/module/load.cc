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

	bool Module::preload() noexcept {

		bool rc = true;

		Config::Value<std::vector<std::string>> modules{"modules","load-at-startup",""};

		if(modules.size()) {

			Logger::String("Preloading ",modules.size()," module(s) from configuration file").trace("module");

			for(std::string &module : modules) {

				try {

					Logger::String("Preloading ",module," from configuration file").trace("module");
					load(File::Path{module});

				} catch(const std::exception &e) {

					cerr << "module\t" << e.what() << endl;
					rc = false;

				} catch(...) {

					cerr << "module\tUnexpected error loading module" << endl;
					rc = false;

				}

			}

		} else {

			Logger::String("Preload list is empty").trace("module");

		}

		return rc;
	}

	void Module::load(const pugi::xml_node &node) {
		Controller::getInstance().load(node);
	}

	bool Module::Controller::load(const std::string &filename, bool required) {

		bool already = false;
		for_each([&already,filename](Module &module) {
			if(!strcasecmp(module.filename().c_str(),filename.c_str())) {
				already = true;
			}
		});

		if(already) {
			return true;
		}

		init(filename,pugi::xml_node{});

		return false;
	}

	void Module::load(const File::Path &path, bool required) {

		path.for_each("*" LIBEXT, [required](const File::Path &path){

			if(Controller::getInstance().load(path,required)) {
				cout << "Module '" << path.c_str() << "' is already loaded" << endl;
			}

			return false;

		},true);

	}

}

