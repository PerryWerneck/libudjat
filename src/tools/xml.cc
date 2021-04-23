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
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <iostream>
 #include <udjat.h>

 using namespace std;
 using namespace pugi;

 namespace Udjat {

	static xml_attribute find(const xml_node &n, const char *name, bool upsearch) {

		auto node = n;

		while(node) {

			xml_attribute attribute = node.attribute(name);
			if(attribute)
				return attribute;

			for(auto child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

				if(strcasecmp(name,child.attribute("name").as_string()) == 0)
					return child.attribute("value");

			}

			if(upsearch && node.attribute("allow-upsearch").as_bool(true))
				node = node.parent();
			else
				break;
		}

		return xml_attribute();
	}

	Attribute::Attribute(const xml_node &n, const char *name, bool upsearch) : xml_attribute(find(n, name, upsearch)), node(n) {
	}

	std::string Attribute::to_string() const {
		return expand(this->node,this->as_string(""));
	}

	std::string expand(const pugi::xml_node &node, const char *str) {

		string text(str);

		expand(text,[node](const char *key){

			Attribute attribute(node,key);
			if(attribute) {
				return attribute.as_string();
			}

			return "${}";

		});

		return text;

	}

 };
