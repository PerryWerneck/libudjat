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
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <cstdarg>
 #include <udjat/tools/quark.h>
 #include <sstream>
 #include <iomanip>

 using namespace std;

 namespace Udjat {

	String::String(const XML::Node &node, const char *attrname, const char *def, bool upsearch) {

		auto attribute = node.attribute(attrname);
		if(attribute) {
			// Found attribute
			assign(attribute.as_string(def ? def : ""));

		} else if(upsearch) {

			// Mount attribute name.
			String upname{node.name()};
			upname += "-";
			upname += attrname;
			bool searching = true;

			debug("Doing an upsearch for '",upname.c_str(),"'");
			for(pugi::xml_node parent = node.parent(); parent  && searching; parent = parent.parent()) {

				// Search for an attribute '${upname}'
				for(pugi::xml_node child = parent.child("attribute"); child && searching; child = child.next_sibling("attribute")) {

					const char * name = child.attribute("name").as_string("");
					if(name && *name && strcasecmp(name,upname.c_str()) == 0) {
						assign(child.attribute("value").as_string(def ? def : ""));
						searching = false;
						break;
					}

				}
			}

		} else if(def) {

			assign(def);

		} else {
			throw runtime_error(Logger::Message("Required attribute '{}' is missing",attrname));
		}

		// And, for last, expand the string.
		if(!empty()) {
			expand(node);
		}

	}

 }

