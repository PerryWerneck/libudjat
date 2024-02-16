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

	Module * Module::Controller::find_by_name(const char *name) {

		for(auto module : objects) {

			if(*module == name) {
				return module;
			}

#ifdef _WIN32
			if(String{module->filename()}.has_suffix((string{name} + LIBEXT).c_str(),true)) {
				return module;
			}
#else
			if(!strcasecmp((string{name} + LIBEXT).c_str(),basename(module->filename().c_str()))) {
				return module;
			}
#endif // _WIN32

		}

		return nullptr;

	}

	bool Module::Controller::load(const XML::Node &node) {

		static const char * attributes[] = {
			"name",
			"altname",
			"fallback-to"
		};

		std::vector<std::string> paths{Module::search_paths()};

		for(const char *attribute : attributes) {

			const char *name = node.attribute(attribute).as_string();

			if(!(name && *name)) {
				continue;
			}

			if(find_by_name(name)) {
				debug("module '",name,"' is already loaded");
				return true;
			}

			string filename = locate(name,paths);

			if(!filename.empty()) {

				if(find_by_filename(filename.c_str())) {
					debug("module '",filename,"' is already loaded");
					return true;
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

					Logger::String{module.c_str(),": ",e.what()}.error("module");
					rc = false;

				} catch(...) {

					Logger::String{"Unexpected errror loading '",module.c_str(),"'"}.error("module");
					rc = false;

				}

			}

		} else {

			Logger::String("Preload list is empty").trace("module");

		}

		return rc;
	}

	void Module::load(const XML::Node &node) {
		Controller::getInstance().load(node);
	}

	bool Module::Controller::load(const std::string &filename, bool required) {

		for(auto module : this->objects) {
			if(!strcasecmp(module->filename().c_str(),filename.c_str())) {
				return true;
			}
		}

		init(filename,XML::Node{});
		return false;

	}

	bool Module::load(const char *name, bool required) {
		return Controller::getInstance().load(name,required);
	}

	bool Module::Controller::load(const char *name, bool required) {

		string filename = locate(name,Module::search_paths());
		if(filename.empty()) {
			if(required) {
				throw std::system_error(ENOENT,std::system_category(),Logger::Message("Cant find module '{}'",name));
			}
			return false;
		}

		if(load(filename,required)) {
			Logger::String{"Module '",filename.c_str(),"' was already loaded"}.trace("module");
			return true;
		}

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

