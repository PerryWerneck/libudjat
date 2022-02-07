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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/object.h>
 #include <udjat/agent.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/expander.h>

 namespace Udjat {

	Alert::Alert(const pugi::xml_node &node, const char *defaults) : Abstract::Alert(node) {

		const char *section = node.attribute("settings-from").as_string(defaults);

		url = getAttribute(node,section,"url","");

		if(!(url && *url)) {
			throw runtime_error("Required attribute 'url' is missing");
		}

		payload = Object::expand(node,section,node.child_value());

		{
			// Get alert action.
			// (Dont use the default getAttribute to avoid the creation of a new 'quark')
			auto attribute = getAttribute(node,"action",true);
			if(attribute) {
				action = HTTP::MethodFactory(attribute.as_string("get"));
			} else {
				action = HTTP::MethodFactory(Config::Value<string>(section,"action","get"));
			}
		}

	}

	void Alert::activate(const char *name, const char *url, const char *action, const char *payload) {
		Abstract::Alert::activate(make_shared<Alert>(name,url,HTTP::MethodFactory(action),payload));
	}

	/*
	const char * Alert::expand(const char *value, const pugi::xml_node &node, const char *section) {

		string text{value};
		Udjat::expand(text, [node,section](const char *key, string &value) {

			auto attr = Udjat::Attribute(node,key,key);
			if(attr) {
				value = attr.as_string();
				return true;
			}

			if(Udjat::Config::hasKey(section,key)) {
				value = Udjat::Config::Value<string>(section,key,"");
				return true;
			}

			return false;

		});

		return Udjat::Quark(text).c_str();

	}
	*/

	std::shared_ptr<Abstract::Alert::Activation> Alert::ActivationFactory(const std::function<void(std::string &str)> &expander) const {
		return make_shared<Activation>(*this,expander);
	}

	Value & Alert::getProperties(Value &value) const noexcept {
		Abstract::Alert::getProperties(value);
		value["url"] = url;
		value["action"] = std::to_string(action);
		return value;
	}

 }

