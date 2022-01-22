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
 #include <udjat/agent.h>
 #include <udjat/tools/threadpool.h>

 namespace Udjat {

	Alert::Alert(const pugi::xml_node &node, const char *defaults) : Abstract::Alert(Quark(node,"name").c_str()) {

		const char *section = node.attribute("settings-from").as_string(defaults);

		url = expand(
				Attribute(node,"url","alert-url")
					.as_string(
						Config::Value<string>(section,"url","")
					),
					node,
					section
				);

		if(!(url && *url)) {
			throw runtime_error("Required attribute 'url' or upstream 'alert-url' are both missing");
		}

		payload = expand(
						node.child_value(),
						node,
						section
					);

		action = HTTP::MethodFactory(
						Attribute(node,"action","alert-action")
						.as_string(
							Config::Value<string>(section,"action","get")
						)
					);

	}

	void Alert::activate(const char *name, const char *url, const char *action, const char *payload) {
		Abstract::Alert::activate(make_shared<Alert>(name,url,HTTP::MethodFactory(action),payload));
	}

	const char * Alert::expand(const char *value, const pugi::xml_node &node, const char *section) {

		string text{value};
		Udjat::expand(text, [node,section](const char *key) {

			auto attr = Udjat::Attribute(node,key,key);
			if(attr) {
				return (string) attr.as_string();
			}

			if(Udjat::Config::hasKey(section,key)) {
				return (string) Udjat::Config::Value<string>(section,key,"");
			}

			return string{"${}"};

		});

		return Udjat::Quark(text).c_str();

	}

	std::shared_ptr<Abstract::Alert::Activation> Alert::ActivationFactory(const std::function<void(std::string &str)> &expander) const {

		class Activation : public Abstract::Alert::Activation {
		private:
			string url;
			HTTP::Method action;
			string payload;

		public:
			Activation(const string &u, const HTTP::Method a, const string &p) : url(u), action(a), payload(p) {
			}

			void emit() const override {
				cout << "alerts\tEmitting '" << url << "'" << endl;
				Protocol::call(url.c_str(),action,payload.c_str());
			}

		};

		string url{this->url};
		string payload{this->payload};

		expander(url);
		expander(payload);

		return make_shared<Activation>(url,action,payload);

	}

 }

