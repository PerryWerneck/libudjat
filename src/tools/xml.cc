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
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/expander.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <iostream>
 #include <cstdarg>

 using namespace std;
 using namespace pugi;

 namespace Udjat {

	static xml_attribute find(const xml_node &n, const char *name, const char *upsearch) {

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
				node = node.parent();
			} else {
				break;
			}
		}

		return xml_attribute();
	}

	Attribute::Attribute(const pugi::xml_node &node, const char *name, const char *upsearch) : xml_attribute(find(node, name, upsearch)) {

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

	Attribute::Attribute(const xml_node &node, const char *name, bool upsearch) : Attribute(node,name,(upsearch ? name : nullptr)) {
	}

	Attribute::Attribute(const pugi::xml_node &node, const char *name) : Attribute(node,name,node.attribute("allow-upsearch").as_bool(true)) {
	}

	std::string Attribute::to_string(const string &def) const {
		if(*this) {
			return def;
		}
		return value;
	}

	Quark Attribute::as_quark(const char *def) const {
		return Quark(to_string(def));
	}

	const char * Attribute::c_str(const char *def) const {
		return as_quark(def).c_str();
	}

	bool is_allowed(const pugi::xml_node &node) {

		const char *str;

		// Test if the attribute requirement is valid.
		str = node.attribute("valid-if").as_string();
		if(str && *str && URL(str).test() != 200) {
			return false;
		}

		// Test if the attribute requirement is not valid.
		str = node.attribute("not-valid-if").as_string();
		if(str && *str && URL(str).test() == 200) {
			return false;
		}

		return true;
	}

	std::string expand(const pugi::xml_node &node, const pugi::xml_attribute &attribute, const char *def) {
		return Udjat::String(attribute.as_string(def)).expand(node);
	}

	std::string expand(const pugi::xml_node &node, const char *str) {

		return Udjat::String(str).expand(node);

		/*
		string text(str);

		expand(text,[node](const char *key, string &value){

			pugi::xml_attribute attribute;

			attribute = Object::getAttribute(node,key);
			if(attribute) {
				value = attribute.as_string();
				return true;
			}

			attribute = Object::getAttribute(node,key,false);
			if(attribute) {
				value = attribute.as_string();
				return true;
			}

			// Not expanded
			return false;

		});
		return text;

		*/


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

 };
