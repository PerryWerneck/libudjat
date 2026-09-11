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
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/expander.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/quark.h>
 #include <iostream>
 #include <cstdarg>

 using namespace std;
 using namespace pugi;

 namespace Udjat {

	bool XML::for_each_attribute(const XML::Node &node, const char *attrname, const std::function<bool(const Udjat::Properties &props)> &test) {
		return node.for_each_attribute(attrname,test);
	}

	bool XML::for_each(const XML::Node &node, const char *attrname, const std::function<bool(const Udjat::Properties &props)> &test) {
		return node.for_each_attribute(attrname,test);
	}


	static XML::Attribute find(const XML::Node &n, const char *name, const char *upsearch) {

		auto node = n;

		while(node) {

			xml_attribute attribute = node.attribute(name);
			if(attribute)
				return attribute;

			for(auto child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

				if(strcasecmp(name,child.attribute("name").as_string()) == 0)
					return child.attribute("value");

			}

			if(upsearch && node.attribute("allow-upsearch").as_bool(true)) {
				name = upsearch;
				node = node.pugi::xml_node::parent();
			} else {
				break;
			}
		}

		return xml_attribute();
	}

	Attribute::Attribute(const XML::Node &node, const char *name, const char *upsearch) : xml_attribute(find(node, name, upsearch)) {

		value = this->as_string();

		expand(value,[node](const char *key, string &value){

			auto attr = find(node,key,key);
			if(attr) {
				value = attr.as_string();
				return true;
			}

			return false;

		});

	}

	Attribute::Attribute(const XML::Node &node, const char *name, bool upsearch) : Attribute(node,name,(upsearch ? name : nullptr)) {
	}

	Attribute::Attribute(const XML::Node &node, const char *name) : Attribute(node,name,node.attribute("allow-upsearch").as_bool(true)) {
	}

	std::string Attribute::to_string(const string &def) const {
		if(*this) {
			return def;
		}
		return value;
	}

	const char * Attribute::c_str(const char *def) const {
		return Quark(to_string(def)).c_str();
	}

	bool is_reserved(const XML::Node &node) {
		return node.reserved();

	}

	bool is_allowed(const XML::Node &node) {
		return node.allowed();

	}
	
	std::string expand(const XML::Node &node, const pugi::xml_attribute &attribute, const char *def) {
		return Udjat::String(attribute.as_string(def)).expand(node);
	}

	std::string expand(const XML::Node &node, const char *str) {

		return Udjat::String(str).expand(node);

	}

	size_t Attribute::select(const char *value, ...) {

		const char * attr = as_string(value);

		size_t index = 0;

		va_list args;
		va_start(args, value);
		while(value) {

			if(!strcasecmp(attr,value)) {
				va_end(args);
				return index;
			}

			index++;
			value = va_arg(args, const char *);
		}
		va_end(args);

		throw system_error(ENOENT,system_category(),Logger::Message("Unexpected value '{}'",attr));

	}

	void XML::options(const XML::Node &node, const std::function<void(const char *name, const char *value)> &call) {

		for(XML::Node child = node.child("option"); child; child = child.next_sibling("option")) {

			const char *name = child.attribute("name").as_string();
			if(!(name && *name)) {
				Logger::String{"Ignoring unnamed option"}.warning("xml");
				continue;
			}

			call(
				name,
				String(child.attribute("value").as_string()).expand(child).c_str()
			);

		}

	}

 };
