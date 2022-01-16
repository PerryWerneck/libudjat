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

 #include "private.h"
 #include <udjat.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>

 namespace Udjat {

	static const char * expand(string &text,const pugi::xml_node &node,const char *section) {

		expand(text, [node,section](const char *key) {

			auto attr = Attribute(node,key,true);
			if(attr) {
				return (string) attr.as_string();
			}

			if(Config::hasKey(section,key)) {
				return (string) Config::Value<string>(section,key,"");
			}

			return string{"${}"};
		});

		return Quark(text).c_str();
	}

	Alert::Settings::Settings(const pugi::xml_node &node) {

		const char *section = node.attribute("settings-from").as_string("alert-defaults");

		name = Quark(node,"name",name).c_str();
		action = Quark(node,"action",action).c_str();

		// Get & expand URL.
		string url =
			Attribute(node,"url")
				.as_string(
					Config::Value<string>(section,"url","")
				);

		if(url.empty()) {
			throw runtime_error("Alert definition requires an URL attribute");
		}

		this->url = expand(url,node,section);

		// Get & expand Payload.
		string text = strip(node.child_value());
		if(!text.empty()) {
			payload = expand(text,node,section);
		}


	}

 }
