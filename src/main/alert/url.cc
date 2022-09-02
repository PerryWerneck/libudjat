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
 #include <udjat/alerts/url.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/string.h>
 #include <udjat/agent.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/expander.h>

 using namespace std;

 namespace Udjat {

	Alert::URL::URL(const pugi::xml_node &node, const char *defaults) : Abstract::Alert(node) {

		const char *section = node.attribute("settings-from").as_string(defaults);

		url = getAttribute(node,section,"url","");

		if(!(url && *url)) {
			throw runtime_error(string{"Required attribute 'url' is missing on alert '"} + name() + "'");
		}

		String child(node.child_value());
		if(getAttribute(node,section,"strip-payload",true)) {
			child.strip();
		}

		payload = Object::expand(node,section,child.c_str());

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

	std::shared_ptr<Abstract::Alert::Activation> Alert::URL::ActivationFactory() const {
		return make_shared<Activation>(this);
	}

	Value & Alert::URL::getProperties(Value &value) const noexcept {
		Abstract::Alert::getProperties(value);
		value["url"] = url;
		value["action"] = std::to_string(action);
		return value;
	}

	Value & Alert::URL::Activation::getProperties(Value &value) const noexcept {
		Abstract::Alert::Activation::getProperties(value);
		value["url"] = url.c_str();
		value["action"] = std::to_string(action);
		return value;
	}

	Alert::URL::Activation::Activation(const Udjat::Alert::URL *alert) : Abstract::Alert::Activation(alert), url(alert->url), action(alert->action), payload(alert->payload) {
		url.expand(*alert,true,false);
		payload.expand(*alert,true,false);
	}

	void Alert::URL::Activation::emit() {

		url.expand();
		payload.expand();

		if(verbose()) {
			if(description.empty()) {
				info() << "Emitting " << action << " " << url << endl << payload << endl;
			} else {
				info() << description << ": " << action << " " << url << endl << payload << endl;
			}
		}

		String response = Protocol::call(url.c_str(),action,payload.c_str());

		if(verbose()) {
			info() << response << endl;
		}

	}

	void Alert::URL::Activation::set(const Abstract::Object &object) {
#ifdef DEBUG
		cout << __FILE__ << "(" << __LINE__ << ")" << endl
				<< "URL='" << url << "'" << endl
				<< "PAYLOAD='" << payload << endl;
#endif // DEBUG
		url.expand(object);
		payload.expand(object);
	}

 }

