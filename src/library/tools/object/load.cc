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
 #include <udjat/defs.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/properties.h>
 #include <algorithm>

 #ifdef HAVE_PUGIXML
 	#include <private/pugixml.h>
 #endif // HAVE_PUGIXML

 using namespace std;

 namespace Udjat {

	time_t Abstract::Object::load(const char *p) {

		time_t next = 0;

#ifdef HAVE_PUGIXML
		File::Path path{XML::PathFactory(p)};
		if(path.dir()) {

			// Is a directory, scan for files
			Logger::String{"Loading xml definitions from directory '",path.c_str(),"'"}.info(name());

			std::vector<std::string> files;
			path.for_each("*.xml",[&files](const File::Path &path) -> bool {
				files.emplace_back(path.c_str());
				return false;
			});

			sort(files.begin(), files.end());

			for(const auto &file : files) {

				// Recursive call to parse document.
				time_t expires = load(file.c_str());
				if(expires) {
					expires += time(0);
					if(expires < next || next == 0) {
						next = expires;
					}
				}

			}

		} else {

			// Is a file, load it
			Logger::String{"Loading xml definitions from file '",path.c_str(),"'"}.info(name());

			XML::Document document{path.c_str()};
			XML::Node root{document.document_element()};
			
			next = TimeStamp{root,"update-timer"};
			if(next) {
				next += time(0);
			}

			append_children(root);

		}
#endif // HAVE_PUGIXML

		if(next) {
			Logger::String{"Next update in ",TimeStamp{next}.to_string().c_str()}.info(name());
		} else {
			Logger::String{"No next update defined"}.trace(name());
		}

		return next;

	}

 }
