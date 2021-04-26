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
 #include <udjat/url.h>
 #include <udjat/factory.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>

 namespace Udjat {

	URLAlert::URLAlert(const pugi::xml_node &node) : Alert(node) {

		string section = getConfigSection(node,"url");

		method =
			Attribute(node,"method",false)
				.as_string(
					Config::Value<string>(section.c_str(),"method",method.c_str()).c_str()
				);

		// Use 'to_string' to expand macros.
		url =
			Attribute(node,"url",false)
				.to_string(
					Config::Value<string>(section.c_str(),"url",url.c_str())
				);

		timeout =
			Attribute(node,"timeout")
				.as_uint(
					Config::Value<unsigned int>(section.c_str(),"timeout",timeout)
				);

		mimetype =
			Attribute(node,"mime-type",false)
				.as_string(
					Config::Value<string>(section.c_str(),"mime-type",mimetype.c_str()).c_str()
				);

	}

	URLAlert::~URLAlert() {
	}

	void URLAlert::activate(const Abstract::Agent &agent, const Abstract::State &state) {

		class Event : public Alert::Event {
		public:

			string url;
			string payload;

			Event(const Abstract::Agent &agent, const Abstract::State &state,string &u) : Alert::Event(agent, state),url(u) {
				info("Event '{}' created",url);
			}

			virtual ~Event() {
				info("Event '{}' destroyed",url);
			}

			const char * getDescription() const override {
				return url.c_str();
			}

			void alert(size_t current, size_t total) override {
				info("Emitting '{}' ({}/{})",this->url,current,total);
				auto response = URL(this->url.c_str()).get();


				if(!response->isValid()) {
					throw runtime_error(to_string(response->getStatusCode()) + " " + response->getStatusMessage());
				}

				if(response->size()) {
					cout << response->c_str() << endl;
				}

			}

		};

		// Expand URL.
		string url = this->url.c_str();
		agent.expand(url);
		state.expand(url);

		// Create event.
		auto event = make_shared<Event>(agent,state,url);

		// Setup event.


		// Activate event.
		Alert::activate(event);
	}


 }
