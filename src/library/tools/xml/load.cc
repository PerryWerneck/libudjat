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

 #ifdef LOG_DOMAIN
 	#undef LOG_DOMAIN
 #endif
 #define LOG_DOMAIN "xml"
 #include <udjat/tools/logger.h>

 #include <udjat/tools/xml.h>
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/intl.h>
 #include <string>
 #include <udjat/tools/string.h>
 #include <udjat/tools/abstract/object.h>
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
 	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	time_t Abstract::Object::parse(const char *p) {

		time_t next = 0;

		File::Path path{XML::PathFactory(p)};
		if(path.dir()) {

			// Is a directory, scan for files
			Logger::String{"Loading xml definitions from directory '",path.c_str(),"'"}.trace();

			std::vector<std::string> files;
			path.for_each("*.xml",[&files](const File::Path &path) -> bool {
				files.emplace_back(path.c_str());
				return false;
			});

			std::sort(files.begin(), files.end());

			for(const auto &file : files) {

				// Recursive call to parse document.
				time_t expires = parse(file.c_str());
				if(expires) {
					expires += time(0);
					if(expires < next || next == 0) {
						next = expires;
					}
				}

			}

		} else {

			// Is a file, load it
			Logger::String{"Loading xml definitions from file '",path.c_str(),"'"}.info();

			XML::Document document{path.c_str()};

			const auto &root = document.document_element();
			next = TimeStamp{root,"update-timer"};
			if(next) {
				next += time(0);
			}

			// Parse the document, create the children.
			for(const XML::Node &node : root) {
				if(!node.attribute("preload").as_bool(false)) {
					parse(node);
				}
			}

		}

#ifdef DEBUG 
		if(next) {
			debug("Next update in ",TimeStamp{next}.to_string());
		} else {
			debug("No next update defined");
		}
#endif // DEBUG		

		return next;

	}

	File::Path XML::PathFactory(const char *p) {

		if(p && *p) {
			return File::Path{p};
		}	

		Config::Value<string> cfg{"application","definitions",""};
		if(!cfg.empty()) {
			return File::Path{cfg.c_str()};
		}

		Application::Name name;
		Application::DataDir datadir{nullptr,false};

		String options[] = {
#ifndef _WIN32
			String{"/etc/",name.c_str(),"/xml"},
			String{"/etc/",name.c_str(),".xml.d"},
#endif // _WIN32
			String{datadir.c_str(),"settings.xml"},
			String{datadir.c_str(),"xml.d"},
		};

		for(const String &option : options) {
			debug("Checking '",option.c_str(),"'");
			if(access(option.c_str(),R_OK) == 0) {
				debug("Found '",option.c_str(),"'");
				return File::Path{option.c_str()};
			}
		}

		throw std::system_error(ENOENT,std::system_category(), _("Configuration file not found"));

	}

	time_t XML::parse(const char *p) {

		File::Path path{XML::PathFactory(p)};

		time_t next = 0;

		if(path.dir()) {

			// Is a directory, scan for files
			std::vector<std::string> files;
			path.for_each("*.xml",[&files](const File::Path &path) -> bool {
				files.emplace_back(path.c_str());
				return false;
			});

			std::sort(files.begin(), files.end());

			for(const auto &file : files) {

				// Recursive call to parse document.
				time_t result = XML::parse(file.c_str());
				if(result && (result < next || next == 0)) {
					next = result;
				}
				
			}

		} else {

			// Is a file, load it
			Logger::String{"Loading xml definitions from '",path.c_str(),"'"}.info();
			next = Document{path.c_str()}.parse();

		}

		return next;

	}

 }