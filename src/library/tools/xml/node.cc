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

 #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/string.h>
 #include <stdexcept>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>

 #ifdef HAVE_VMDETECT
	#include <vmdetect/virtualmachine.h>
 #endif // HAVE_VMDETECT

 using namespace std;

 namespace Udjat {

	XML::Node::~Node() {
	}

	const char * XML::Node::node_name() const noexcept {
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

	bool XML::Node::reserved() const noexcept {
		if(!(strncasecmp(node_name(),"attribute",9))) {
			return true;
		}
		return false;
	}

	bool XML::Node::allowed() const noexcept {

		if(reserved()) {
			return false;
		}

#ifdef _WIN32

		if(!attribute("allowed-in-windows").as_bool(true)) {
			return false;
		}

#else

		if(!pugi::xml_node::attribute("allowed-in-linux").as_bool(true)) {
			return false;
		}

#endif // _WIN32

#ifdef HAVE_VMDETECT

		if(!(pugi::xml_node::attribute("allowed-in-virtual-machine").as_bool(true) || VirtualMachine{Logger::enabled(Logger::Debug)}) ) {
			return false;
		}

#else

		if(!pugi::xml_node::attribute("allowed-in-virtual-machine").as_bool(true)) {
			Logger::String{"Library built without virtual machine support, ignoring 'allowed-in-virtual-machine' attribute"}.error();
		}

#endif // HAVE_VMDETECT

		if(XML::test(*this, "valid-if", false) || (XML::test(*this, "allow-if", false))) {
			return true;
		}

		/*
		// Test if the attribute requirement is valid.
		str = node.attribute("valid-if").as_string();
		if(str && *str && URL{str}.test() != 200) {
			return false;
		}

		str = node.attribute("allow-if").as_string();
		if(str && *str && URL{str}.test() != 200) {
			return false;
		}

		// Test if the attribute requirement is not valid.
		str = node.attribute("not-valid-if").as_string();
		if(str && *str && URL{str}.test() == 200) {
			return false;
		}

		str = node.attribute("invalid-if").as_string();
		if(str && *str && URL{str}.test() == 200) {
			return false;
		}

		str = node.attribute("ignore-if").as_string();
		if(str && *str && URL{str}.test() == 200) {
			return false;
		}

		str = node.attribute("deny-if").as_string();
		if(str && *str && URL{str}.test() == 200) {
			return false;
		}
		*/

		return true;
	}

	/// @brief Scan XML node and parents from node 'attribute'
	/// @param node The starting point.
	/// @param attrname The required attribute name.
	/// @return The node (empty if not found).
	static const pugi::xml_node xml_attribute_node(const XML::Node &node, const char *attrname) {

		// Check children.
		for(auto child = node.pugi::xml_node::child("attribute"); child; child = child.next_sibling("attribute")) {
			if(!strcasecmp(child.attribute("name").as_string("*"),attrname)) {
				return child;
			}
		}

		// Check parents.
		String parent_name{node.pugi::xml_node::name(),"-",attrname};
		for(auto parent = node.pugi::xml_node::parent(); parent; parent = parent.parent()) {

			for(auto child = parent.child("attribute"); child; child = child.next_sibling("attribute")) {
				const char *name = child.attribute("name").as_string("*");
				if(!(strcasecmp(name,attrname) && strcasecmp(name,parent_name.c_str()))) {
					return child;
				}
			}
		}

		return pugi::xml_node();
	}

	/// @brief Scan XML node and parents for attribute.
	/// @param node The starting point.
	/// @param attrname The required attribute name.
	/// @return The attribute (empty if not found).
	static const pugi::xml_attribute xml_attribute(const XML::Node &node, const char *attrname) {

		// debug("Searching for attribute '",attrname,"' in node '",node.node_name(),"'");

		// Check for standard attribute.
		{
			auto attr = node.pugi::xml_node::attribute(attrname);
			if(attr) {
				return attr;
			}
		}

		// Check children.
		for(auto child = node.pugi::xml_node::child("attribute"); child; child = child.next_sibling("attribute")) {
			if(!strcasecmp(child.attribute("name").as_string("*"),attrname)) {
				return child.attribute("value");
			}
		}

		// Check parents.
		String parent_name{node.pugi::xml_node::name(),"-",attrname};
		for(auto parent = node.pugi::xml_node::parent(); parent; parent = parent.parent()) {

			{
				auto attr = parent.attribute(parent_name.c_str());
				if(attr) {
					return attr;
				}
			}

			for(auto child = parent.child("attribute"); child; child = child.next_sibling("attribute")) {
				const char *name = child.attribute("name").as_string("*");
				if(!(strcasecmp(name,attrname) && strcasecmp(name,parent_name.c_str()))) {
					return child.attribute("value");
				}
			}
		}

		// debug("Cant find attribute '",attrname,"'");
		return pugi::xml_attribute();
	}

	const String XML::Node::get(const char *attrname, const char *def) const {

		auto attr = xml_attribute(*this,attrname);
		if(attr) {
			debug("Found attribute '",attrname,"' for node '",node_name(),"'");
			return attr.as_string(def);
		}

		if(!def) {
#ifdef BUILD_LEGACY
			throw logic_error(Logger::String{"The required attribute '",attrname,"' is missing"});
#else
			throw logic_error(Logger::String{"Required attribute '",attrname,"' is missing at ",pugi::xml_node::path()});
#endif
		}
		return def;

	}

	bool XML::Node::contains(const char *name) const noexcept {
		if(xml_attribute(*this,name)) {
			return true;
		}
		return false;
	}

	bool XML::Node::has_child(const char *name) const noexcept {
		return pugi::xml_node::child(name);
	}

	String XML::Node::path() const noexcept {
#ifdef BUILD_LEGACY
		return Properties::path();
#else
		return pugi::xml_node::path();
#endif
	}

	bool XML::Node::get(const char *attrname, const bool def) const {
		return xml_attribute(*this,attrname).as_bool(def);
	}

	double XML::Node::get(const char *attrname, const double def) const {
		return xml_attribute(*this,attrname).as_double(def);
	}

	float XML::Node::get(const char *attrname, const float def) const {
		return xml_attribute(*this,attrname).as_float(def);
	}

	int XML::Node::get(const char *attrname, const int def) const {
		return xml_attribute(*this,attrname).as_int(def);
	}

	unsigned int XML::Node::get(const char *attrname, const unsigned int def) const {
		return xml_attribute(*this,attrname).as_uint(def);
	}
	
	String XML::Node::child_value() const {
		return pugi::xml_node::child_value();
	}

	String XML::Node::child_value(const char *attrname, const char *def) const {
		String response = xml_attribute_node(*this,attrname).child_value();
		if(!response.empty()) {
			return response;
		}
		return def;
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

		// Scan for nodes.
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

			debug("nome_name=",node_name.c_str()," group_name=",group_name.c_str());

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