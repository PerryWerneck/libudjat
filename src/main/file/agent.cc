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

namespace Udjat {

	File::Agent::Agent(const Quark &n) : name(n) {
		Controller::getInstance().insert(this);
	}

	File::Agent::Agent(const char *name) : Agent(Quark(name)) { }

	File::Agent::Agent(const pugi::xml_node &node) : Agent(Quark(node.attribute("path"))) { }

	File::Agent::Agent(const pugi::xml_attribute &attribute) : Agent(Quark(attribute)) { }

	File::Agent::~Agent() {
		Controller::getInstance().remove(this);
	}

	void File::Agent::set(const char *contents) {

		cout << "--- " << name << " ---" << endl << contents << endl << "---" << endl;
	}

}
