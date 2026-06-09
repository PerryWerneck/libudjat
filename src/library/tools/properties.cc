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
 #include <udjat/tools/logger.h>

 namespace Udjat {


	const char * Properties::NameFactory() const noexcept {

		String name{get("name")};

		if(name.empty()) {
			Logger::String{"<",node_name(),"> doesn't have the required attribute 'name', using default '",name,"'"}.trace("properties");
			name.assign(node_name());
		}

		return name.as_quark();

	}

	const char *Properties::node_name() const noexcept {
		return "unnamed";
	}

	const String Properties::get(const char *attrname, const char *def) const {
		return String{def ? def : ""}; // No requred attributes check in default properties.
	}
	
	String Properties::operator[](const char *attrname) const {
		return get(attrname);
	}

	String Properties::child_value() const {
		return "";
	}

	bool Properties::for_each_child(const std::function<bool(const Properties &property)> &call) const {
		// The default properties have no children.
		return false;
	}

	bool Properties::for_each_child(const char *name, const std::function<bool(const Properties &property)> &call) const {
		return for_each_child([&name,&call](const Properties &property) {
			if(!strcasecmp(name,property.node_name())) {
				return call(property);
			}
			return false;
		});
	}

	bool Properties::for_each_child(const char *tagname, const char *, const std::function<bool(const Udjat::Properties &property)> &call) const {
		return for_each_child(tagname,[&call](const Properties &property) {
			return call(property);
		});
	}

	bool Properties::for_each_attribute(const char *, const std::function<bool(const Udjat::Properties &props)> &) const {
		return false;
	}

 }
