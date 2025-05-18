/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/defs.h>

 #define LOG_DOMAIN "xml"
 #include <udjat/tools/logger.h>

 #include <udjat/tools/xml.h>
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <string>
 #include <udjat/tools/string.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	time_t XML::load(const char *p) {

		File::Path path;

		if(p && *p) {

			// Use requested path
			path.assign(p);

		} else {

			// No path, search for one.
			Config::Value<string> cfg{"application","definitions",""};
			if(!cfg.empty()) {

				path.assign(cfg);

			} else {

				Application::Name name;

				String options[] = {
#ifndef _WIN32
					String{"/etc/",name.c_str(),"/xml"},
					String{"/etc/",name.c_str(),".xml.d"},
#endif // _WIN32
					String{Application::DataDir{}.c_str(),"settings.xml"},
					String{Application::DataDir{}.c_str(),"xml.d"}

				};

				for(const String &option : options) {
					path.assign(option.c_str());
					debug("Searching for '",path.c_str(),"'");
					if(path) {
						break;
					}

				}

			}

		}

		if(!path) {
			throw std::system_error(ENOENT,std::system_category(),"Cant load configuration");
		}

		time_t next = 0;

		if(path.dir()) {

			// Is a directory, scan for files
			path.for_each("*.xml",[&next](const File::Path &path) -> bool {
				time_t result = XML::load(path.c_str());
				if(result && (result < next || next == 0)) {
					next = result;
				}
				return false;
			});

		} else {

			// Is a file, load it
			Logger::String{"Loading xml definitions from '",path.c_str(),"'"}.trace("settings");
			next = Document{path.c_str()}.ObjectFactory();

		}

		return next;

	}

 }