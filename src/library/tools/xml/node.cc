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
 #include <stdexcept>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

	XML::Node::~Node() {
	}

	const char * XML::Node::name() const noexcept {
		return pugi::xml_node::name();
	}

	XML::Node XML::Node::parent() const {
		return Node{pugi::xml_node::parent()};
	}

	XML::Node XML::Node::child(const char *name) const {
		return Node{pugi::xml_node::child(name)};
	}

	XML::Node XML::Node::next_sibling(const char *name) const {
		return Node{pugi::xml_node::next_sibling(name)};
	}

	const String XML::Node::get(const char *attrname, const char *def) const {
		auto attr = XML::AttributeFactory(*this,attrname);
		if(attr) {
			return attr.as_string(def);
		}
		if(!def) {
			throw std::logic_error(Logger::String{"The required attribute '",attrname,"' is missing"});
		}
		return def;

	}
	
	String XML::Node::child_value() const {
		return pugi::xml_node::child_value();
	}

	bool XML::Node::for_each_child(const char *tagname, const std::function<bool(const Properties &property)> &call) const {		
		for(auto child = this->child(tagname); child; child = child.next_sibling(tagname)) {
			if(call(XML::Node{child})) {
				return true;
			}
		}
		return false;
	}

	bool XML::Node::for_each_child(const std::function<bool(const Properties &property)> &call) const {
		for(auto child : *this) {
			if(call(XML::Node{child})) {
				return true;
			}
		}
		return false;
	}

	bool XML::Node::for_each_attribute(const char *attrname, const std::function<bool(const Udjat::Properties &props)> &test) const {
		for(XML::Node nd = *this; nd; nd = nd.parent()) {
			for(XML::Node child = nd.child(attrname); child; child = child.next_sibling(attrname)) {
				if(is_allowed(child) && test(child)) {
					return true;
				}
			}
		}
		return false;
	}

	bool XML::Node::for_each_child(const char *tagname, const char *group, const std::function<bool(const Properties &property)> &call) const {

		for(auto node = pugi::xml_node::child(tagname); node; node = node.pugi::xml_node::next_sibling(tagname)) {
			if(call(XML::Node{node})) {
				return true;
			}
		}

		if(group && *group) {

			string group_name{pugi::xml_node::name()};
			group_name += '-';
			group_name += group;

			string node_name{pugi::xml_node::name()};
			node_name += '-';
			node_name += tagname;

			for(auto parent = pugi::xml_node::parent(); parent; parent = parent.parent()) {

				// Scan for nodes.
				for(auto node = parent.child(node_name.c_str()); node; node = node.next_sibling(node_name.c_str())) {
					if(call(XML::Node{node})) {
						return true;
					}
				}

				// Scan for groups.
				for(auto grp = parent.pugi::xml_node::child(group_name.c_str()); grp; grp = grp.next_sibling(group_name.c_str())) {

					for(auto node = grp.pugi::xml_node::child(tagname); node; node = node.next_sibling(tagname)) {
						if(call(XML::Node{node})) {
							return true;
						}
					}

				}

			}

		}

		return false;

	}

 }