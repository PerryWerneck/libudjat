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
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/activatable.h>
 #include <udjat/tools/string.h>
 #include <vector>
 #include <cstdarg>

 using namespace std;

 namespace Udjat {

	Activatable::Activatable(const XML::Node &node) : object_name{String{node,"name"}.as_quark()} {
	}

	Activatable::~Activatable() {
	}

	bool Activatable::activate(const Udjat::Abstract::Object &) noexcept {
		return activate();
	}

	bool Activatable::deactivate() noexcept {
		return false;	// Allways return false if the object cant be deactivated.
	}

	const char * Activatable::payload(const XML::Node &node) {
		String child(node.child_value());
		if(child.empty()) {
			child = node.attribute("payload").as_string();
		}
		child.expand(node);
		if(node.attribute("strip-payload").as_bool(true)) {
			child.strip();
		}
		return child.as_quark();
	}

 }
