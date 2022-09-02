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
 #include <udjat/alerts/abstract.h>
 #include <udjat/agent.h>
 #include <iostream>
 #include <udjat/moduleinfo.h>

 using namespace std;

 namespace Udjat {

	bool Factory::search(const pugi::xml_node &node, const std::function<bool(Factory &, const pugi::xml_node &)> &call, const char *typeattribute) {

		if(!typeattribute) {
			typeattribute = Object::getAttribute(node,(string{node.name()} + "-defaults").c_str(),"type","default");
		}

#ifdef DEBUG
		cout << "factories\tSearching for '" << typeattribute << "'" << endl;
#endif // DEBUG

		if(strcasecmp(typeattribute,"default")) {
			//
			// It's NOT the default factory, search by name.
			//
			return for_each(typeattribute,[&call,node](Factory &factory){

				try {

					return call(factory,node);

				} catch(const std::exception &e) {

					factory.error() <<  e.what() << endl;

				} catch(...) {

					factory.error() << "Unexpected error" << endl;

				}

				return false;

			});

		}

		//
		// Asking for the default factory, search for one.
		//

		int level = (int) Object::getAttribute(node,string{node.name()} + "-defaults","upstream-levels",(unsigned int) 3);
		auto alertnode = node;

		while(alertnode && alertnode.attribute("allow-upstream").as_bool(true) && level-- > 0) {

//#ifdef DEBUG
//			cout << "factories\tSearching on '" << alertnode.name() << "'" << endl;
//#endif // DEBUG

			Factory * factory = Factory::find(alertnode.name());
			if(factory && call(*factory,node)) {
				return true;
			}
			alertnode = alertnode.parent();
		}

		return false;

	}

 }
