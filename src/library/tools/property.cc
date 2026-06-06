/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <functional>
 #include <udjat/tools/string.h>
 #include <udjat/tools/properties.h>

 namespace Udjat {


	const char *Property::name() const noexcept {
		return "unnamed";
	}

	const String Property::get(const char *attrname, const char *def) const {
		return String{def};
	}
	
	String Property::operator[](const char *attrname) const {
		return get(attrname);
	}

	String Property::child_value() const {
		return "";
	}

	bool Property::for_each(const char *name, const std::function<bool(const Property &property)> &call) const {
		return for_each([&name,&call](const Property &property) {
			if(!strcasecmp(name,property.name())) {
				return call(property);
			}
			return false;
		});
	}

	bool Property::for_each(const std::function<bool(const Property &property)> &call) const {
		return false;
	}

 }
