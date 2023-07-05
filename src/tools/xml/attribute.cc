/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <cstring>

 using namespace std;
 using namespace pugi;

 namespace Udjat {

	const Quark XML::QuarkFactory(const XML::Node &node, const char *aname, const char *vname, const char *def) {
		return StringFactory(node,aname,vname,def).as_quark();
	}

	String XML::StringFactory(const XML::Node &node, const char *aname, const char *vname, const char *def) {

		// Search for <node ${aname}="value"
		{
			const pugi::xml_attribute &attr = node.attribute(aname);
			if(attr) {
				return String{attr.as_string(def)}.expand(node);
			}
		}

		// Scan upper nodes.
		string upname{node.name()};
		upname += "-";
		upname += aname;

		const char *attrname = aname;
		for(pugi::xml_node parent = node; parent ; parent = parent.parent()) {

			// Search for <attribute name='${attrname}' ${vname}="value" />
			for(pugi::xml_node child = parent.child("attribute"); child; child = child.next_sibling("attribute")) {

				const char * name = child.attribute("name").as_string("");

				// is_allowed should be the last test since it can trigger a network request.
				if(name && *name && strcasecmp(name,attrname) == 0 && is_allowed(child)) {
					return String{child.attribute(vname).as_string(def)}.expand(node);
				}

			}

			// Will get up node, replace attrname
			attrname = upname.c_str();
		}

		return String{def}.expand(node);

	}

 }

