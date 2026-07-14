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
 #include <udjat/tools/container.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/url.h>
 #include <string>

 #ifdef HAVE_VMDETECT
	#include <vmdetect/virtualmachine.h>
 #endif // HAVE_VMDETECT

 using namespace std;

 namespace Udjat {

	static Container<Properties::ObjectBuilder> & Factories() {
		static Container<Properties::ObjectBuilder> instance;
		return instance;
	}

	bool Properties::build(const Properties &props) {

		const char *name = props.node_name();
	
		for(const auto factory : Factories()) {
			if(*factory == name) {

				if(!factory->build(props)) {
					continue; // Not handled.
				}
				return true; // Handled.
			}
		}
		return false; // Not handled.
	}

	Properties::ObjectBuilder::ObjectBuilder(const char *name) : builder_name{name} {
		Logger::String{"Registering parser for Properties::",builder_name}.trace();
		Factories().push_back(this);
	}

	Properties::ObjectBuilder::~ObjectBuilder() {
		Logger::String{"Unregistering parser for Properties::",builder_name}.trace();
		Factories().remove(this);
	}

	const char * Properties::NameFactory() const noexcept {

		String name{get("name")};

		if(name.empty()) {
			name.assign(node_name());
			Logger::String{"<",node_name(),"> doesn't have the required attribute 'name', using default '",name.c_str(),"'"}.trace("properties");
		}

		return name.as_quark();

	}

	bool Properties::reserved() const noexcept {
		return false;
	}

	/// @brief Test expression.
	/// @param expression The expression to test.
	/// @return True if the expression was validated and the node is allowed.
	static bool is_allowed(String &expression) {

		expression.strip();

		bool deny = false;

		if(expression[0] == '!') {
			deny = !deny;
			expression.erase(0,1);
			expression.strip();
		}

		if(expression.has_prefix("not ")) {
			deny = !deny;
			expression.erase(0,4);
			expression.strip();
		}

		if(expression.find("://") != string::npos) {
			if(URL{expression.c_str()}.test() != 200) {
				return deny;
			}
		}

		if(!strcasecmp(expression.c_str(),"virtual-machine")) {
#ifdef HAVE_VMDETECT
			if(!VirtualMachine{Logger::enabled(Logger::Debug)}) {
				return deny;
			}
#else
			Logger::String{"Library built without virtual machine support, ignoring 'allowed-if=virtual-machine' attribute"}.error();
#endif
		}

#ifdef _WIN32
		if(!strcasecmp(expression.c_str(),"linux")) {
			return deny;
		}
#else
		if(!strcasecmp(expression.c_str(),"windows")) {
			return deny;
		}
#endif

		return !deny;
	}

	bool Properties::allowed() const noexcept {

		if(reserved()) {
			return false;
		}

		auto expression = get("allowed-if");
		if(!expression.empty()) {
			return is_allowed(expression);
		}

		return true;
	}

	const char *Properties::node_name() const noexcept {
		return "unnamed";
	}

	bool Properties::has_child(const char *) const noexcept {
		return false;
	}

	/// @brief Get properties path (for messages)
	String Properties::path() const noexcept {
		return "properties";
	}

	bool Properties::contains(const char *, bool) const noexcept {
		// Default properties doesnt contains anything.
		return false;
	}

	const String Properties::get(const char *, const char *def) const {
		return String{def ? def : ""}; // No requred attributes check in default properties.
	}
	
	bool Properties::get(const char *attrname, const bool def) const {
		return get(attrname,"").as_bool(def);
	}

	double Properties::get(const char *attrname, const double def) const {
		return get(attrname,"").as_double(def);
	}

	float Properties::get(const char *attrname, const float def) const {
		return get(attrname,"").as_float(def);
	
	}

	int Properties::get(const char *attrname, const int def) const {
		return get(attrname,"").as_int(def);
	}

	unsigned int Properties::get(const char *attrname, const unsigned int def) const {
		return get(attrname,"").as_uint(def);
	}

	String Properties::get(const char *groupname, const char *attrname, const char * def) const {
		if(contains(attrname)) {
			return get(attrname,def);
		}
		return Config::get(groupname,attrname,def);
	}

	bool Properties::get(const char *groupname, const char *attrname, const bool def) const {
		if(contains(attrname)) {
			return get(attrname,def);
		}
		return Config::get(groupname,attrname,def);
	}

	double Properties::get(const char *groupname, const char *attrname, const double def) const {
		if(contains(attrname)) {
			return get(attrname,def);
		}
		return Config::get(groupname,attrname,def);
	}

	float Properties::get(const char *groupname, const char *attrname, const float def) const {
		if(contains(attrname)) {
			return get(attrname,def);
		}
		return Config::get(groupname,attrname,def);
	}

	int Properties::get(const char *groupname, const char *attrname, const int def) const {
		if(contains(attrname)) {
			return get(attrname,def);
		}
		return Config::get(groupname,attrname,def);
	}

	unsigned int Properties::get(const char *groupname, const char *attrname, const unsigned int def) const {
		if(contains(attrname)) {
			return get(attrname,def);
		}
		return Config::get(groupname,attrname,def);
	}

	String Properties::operator[](const char *attrname) const {
		return get(attrname);
	}

	String Properties::child_value() const {
		return "";
	}

	String Properties::child_value(const char *, const char *def) const {
		return def;
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
