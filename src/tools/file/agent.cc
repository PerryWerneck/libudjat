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

	File::Agent::Agent(const Quark &name) {

		watcher = Watcher::insert(this, name, [this](const Udjat::File::Text &file) {
			this->set(file);
		});

	}

	File::Agent::Agent(const char *name) {

		watcher = Watcher::insert(this, name, [this](const Udjat::File::Text &file) {
			this->set(file);
		});

	}

	File::Agent::Agent(const pugi::xml_node &node, const char *name) : Agent(Udjat::Attribute(node,name).as_string()) { }

	File::Agent::Agent(const pugi::xml_node &node) : Agent(Udjat::Attribute(node,"filename").as_string()) { }

	File::Agent::Agent(const pugi::xml_attribute &attribute) : Agent(Quark(attribute)) { }

	File::Agent::~Agent() {
		watcher->remove(this);
	}

	void File::Agent::set(const File::Text &file) {
		set(file.c_str());
	}

	void File::Agent::set(const char *contents) {

#ifdef DEBUG
		cout << "--- " << c_str() << " ---" << endl << contents << endl << "---" << endl;
#endif // DEBUG

	}

}
