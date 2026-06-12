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

#include <udjat/tools/xml.h>

namespace Udjat {

	Module * Module::Controller::find_by_name(const char *name) {

		for(const auto module : modules) {

			if(module->module_name && *module->module_name && strcasecmp(module->module_name,name)) {
				return module;
			}

		}

		return nullptr;

	}

	bool Module::Controller::parse(const XML::Node &node) {

		static const char * attributes[] = {
			"name",
			"altname",
			"path",
			"fallback-to"
		};

		std::vector<std::string> paths{Module::search_paths()};

		for(const char *attribute : attributes) {

			const char *name = node.attribute(attribute).as_string();

			if(!(name && *name)) {
				continue;
			}

			if(*name == '.' || *name == '/') {
				load(name, node);
				return true;
			}

			string filename = locate(name,paths);			
			if(!filename.empty()) {
				load(filename, node);
				return true;
			}

		}

		// Not found.
		if(node.attribute("required").as_bool(true)) {
			throw runtime_error(string{"Cant load required module '"} + node.attribute(attributes[0]).as_string() + "'");
		} else {
			Logger::String{"Cant load module '",node.attribute(attributes[0]).as_string(),"', ignoring"}.warning();
		}

		return true;
	}

	bool Module::load(const std::string &filename, const XML::Node &node) {
		return Controller::getInstance().load(filename,node);
	}

}

