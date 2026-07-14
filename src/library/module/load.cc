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
#define LOG_DOMAIN "module"

#include <config.h>
#include <private/module.h>
#include <sys/types.h>
#include <dirent.h>
#include <udjat/tools/file.h>
#include <udjat/tools/application.h>
#include <udjat/tools/configuration.h>
#include <udjat/tools/object.h>
#include <udjat/tools/logger.h>

#include <private/pugixml.h>

namespace Udjat {

	Module * Module::Controller::find_by_name(const char *name) {

		for(const auto module : modules) {

			if(module->module_name && *module->module_name && strcasecmp(module->module_name,name)) {
				return module;
			}

		}

		return nullptr;

	}

	bool Module::Controller::build(const Properties &props) {
		return load(props);
	}

	bool Module::Controller::load(const Properties &props) {

		static const char * attributes[] = {
			"name",
			"altname",
			"path",
			"fallback-to"
		};

		string detected_name;

		std::vector<std::string> paths{Module::search_paths()};

		for(const char *attribute : attributes) {

			const auto name = props[attribute];

			if(name.empty()) {
				continue;
			}

			detected_name = name;
			if(find_by_name(name.c_str())) {
				return true;
			}

#ifndef LIBUDJAT_STATIC
			if(name[0] == '.' || name[0] == '/') {
				load(name.c_str(), props);
				return true;
			}

			string filename = locate(name.c_str(),paths);			
			if(!filename.empty()) {
				load(filename, props);
				return true;
			}
#endif // !LIBUDJAT_STATIC

		}

		// Invalid
		if(detected_name.empty()) {
			throw runtime_error(String{"Required attribute 'name' is missing or invalid at '",props.path().c_str(),"'"});
		}

		// Not found.
		if(props.get("required",true)) {
#ifdef LIBUDJAT_STATIC
			throw logic_error(String{"Required module '",detected_name,"' is unavailable"});
#else
			throw runtime_error(String{"Cant load required module '",detected_name.c_str(),"'"});
#endif
		} else {
			Logger::String{"Cant load module '",detected_name.c_str(),"', ignoring"}.warning();
		}

		return true;
	}

	bool Module::load(const Properties &props) {
		return Controller::getInstance().load(props);
	}

}

