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
 #include <udjat/factory.h>
 #include <udjat/alert.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>
 #include <iostream>
 #include <udjat/moduleinfo.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::Alert> AlertFactory(const pugi::xml_node &node, const char *name) {

		std::shared_ptr<Abstract::Alert> alert;

		if(!name) {
			name = Object::getAttribute(node,"alert-defaults","type","default");
		}

#ifdef DEBUG
		cout << "alerts\tCreating alert '" << name << "'" << endl;
#endif // DEBUG

		if(strcasecmp(name,"default")) {

			// It's not the default alert, search for factory.
			if(!Factory::for_each(name,[&alert,node](const Factory &factory){

				try {

					alert = factory.AlertFactory(node);
					if(alert) {
						return true;
					}

				} catch(const std::exception &e) {

					factory.error() << "Error '" << e.what() << "' creating alert" << endl;

				} catch(...) {

					factory.error() << "Unexpected error creating alert" << endl;

				}

				return false;

			})) {

				cerr << "alerts\tUnable to create the required alert" << endl;

			}

		} else {

			// It's the default alert, search for a valid factory, first, search upstream.
			int level = (int) Object::getAttribute(node,"alert-defaults","upstream-levels",(unsigned int) 3);
			auto alertnode = node;
			while(alertnode && alertnode.attribute("allow-upstream").as_bool(true) && level-- > 0) {

#ifdef DEBUG
				cout << "alerts\tSearching for alerts on '" << alertnode.name() << "'" << endl;
#endif // DEBUG
				const Factory * factory = Factory::find(alertnode.name());
				if(factory) {
					alert = factory->AlertFactory(node);
					if(alert) {
						cout << "alerts\tUsing alert engine from '" << factory->name() << "'" << endl;
						break;
					}
				}
				alertnode = alertnode.parent();
			}

			if(!alert) {
				cout << "alerts\tUsing the default alert engine" << endl;
				alert = make_shared<Udjat::Alert>(node);
			}

		}

		return alert;

	}

 }
