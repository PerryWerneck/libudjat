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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/properties.h>
 #include <pugixml.hpp>
 #include <udjat/tools/string.h>

 namespace Udjat {

	XML::Node::~Node() {
	}

	const char * XML::Node::name() const noexcept {
		return pugi::xml_node::name();
	}

	XML::Node XML::Node::parent() const {
		return Node{parent()};
	}

	XML::Node XML::Node::child(const char *name) const {
		return Node{child(name)};
	}

	XML::Node XML::Node::next_sibling(const char *name) const {
		return Node{next_sibling(name)};
	}

	const String XML::Node::get(const char *attrname, const char *def = "") const {
		return XML::AttributeFactory(*this,attrname).as_string(def);
	}
	
	String XML::Node::child_value() const {
		return pugi::xml_node::child_value();
	}

	bool XML::Node::for_each(const char *name, const std::function<bool(const Property &property)> &call) const {		
		for(auto child = this->child(name); child; child = child.next_sibling(name)) {
			if(call(XML::Node{child})) {
				return true;
			}
		}
		return false;
	}

	bool XML::Node::for_each(const std::function<bool(const Property &property)> &call) const {
		for(auto child : *this) {
			if(call(XML::Node{child})) {
				return true;
			}
		}
	}

 }