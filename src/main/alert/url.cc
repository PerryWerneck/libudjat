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

		url =
			Attribute(node,"url",false)
				.as_string(
					Config::Value<string>(section.c_str(),"url",url.c_str()).c_str()
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
			Event() : Alert::Event(Quark::getFromStatic("fix-me")) {
				cout << "Event was created" << endl;
			}

			virtual ~Event() {
				cout << "Event was destroyed" << endl;
			}

			void alert() override {
				cout << "Emitting URL Alert" << endl;
			}

		};

		Alert::activate(make_shared<Event>());

	}


 }
