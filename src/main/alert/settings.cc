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

		// Seconds to wait before first activation.
		start =
			Attribute(node,"delay-before-start")
				.as_uint(
					Config::Value<uint32_t>(section,"delay-before-start",start)
				);

		// Seconds to wait on every try.
		interval =
			Attribute(node,"delay-before-retry")
				.as_uint(
					Config::Value<uint32_t>(section,"delay-before-retry",interval)
				);

		// How many success emissions after deactivation or sleep?
		limits.min =
			Attribute(node,"min-retries")
				.as_uint(
					Config::Value<uint32_t>(section,"min-retries",limits.min)
				);

		// How many retries (success+fails) after deactivation or sleep?
		limits.max =
			Attribute(node,"max-retries")
				.as_uint(
					Config::Value<uint32_t>(section,"max-retries",limits.max)
				);

		// How many seconds to restart when failed?
		restart.failed =
			Attribute(node,"restart-when-failed")
				.as_uint(
					Config::Value<uint32_t>(section,"delay-when-failed",restart.failed)
				);

		// How many seconds to restart when suceeded?
		restart.success =
			Attribute(node,"restart-when-succeeded")
				.as_uint(
					Config::Value<uint32_t>(section,"restart-when-succeeded",restart.success)
				);



	}

 }
