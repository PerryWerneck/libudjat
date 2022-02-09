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

	static const Udjat::ModuleInfo moduleinfo { "Alert factory" };

	Alert::Factory::Factory() : Udjat::Factory("alert",moduleinfo) {
	}

	bool Alert::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		try {

			parent.append_alert(node);

		} catch(const std::exception &e) {

			cerr << "alerts\t" << e.what() << endl;
			return false;

		}
		return true;
	}

	bool Alert::Factory::parse(Abstract::State &parent, const pugi::xml_node &node) const {
		parent.append(make_shared<Alert>(node));
		return true;
	}

 }
