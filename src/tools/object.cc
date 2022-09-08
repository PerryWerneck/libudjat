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
 #include <udjat/tools/quark.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/expander.h>
 #include <udjat/factory.h>

 using namespace std;

 namespace Udjat {

	NamedObject::NamedObject(const pugi::xml_node &node) : NamedObject(Quark(node.attribute("name").as_string("unnamed")).c_str()) {
	}

	bool NamedObject::set(const pugi::xml_node &node) {
		if(!(objectName && *objectName)) {
			objectName = Quark(node.attribute("name").as_string(objectName)).c_str();
			return true;
		}
		return false;
	}

	const char * NamedObject::c_str() const noexcept {
		return (this->objectName ? this->objectName : "" );
	}

	int NamedObject::compare(const NamedObject &object ) const {
		return strcasecmp(this->c_str(),object.c_str());
	}

	bool NamedObject::operator==(const char *name) const noexcept {
		return strcasecmp(c_str(), name) == 0;
	}

	bool NamedObject::operator==(const pugi::xml_node &node) const noexcept {
		return strcasecmp(c_str(),node.attribute("name").as_string()) == 0;
	}

	Value & NamedObject::getProperties(Value &value) const noexcept {
		value["name"] = objectName;
		return value;
	}

	std::string NamedObject::to_string() const {
		return c_str();
	}

	Object::Object(const pugi::xml_node &node) : NamedObject(node) {
		set(node);
	}

	void Abstract::Object::setup(const pugi::xml_node &node) {

		for(pugi::xml_node child : node) {

			Factory::for_each(child.name(),[this,&child](Factory &factory) {

				try {

					return factory.push_back(*this,child);

				} catch(const std::exception &e) {

					factory.error() << "Error '" << e.what() << "' parsing node <" << child.name() << ">" << endl;

				} catch(...) {

					factory.error() << "Unexpected error parsing node <" << child.name() << ">" << endl;

				}

				return false;

			});

		}

	}

	void Object::set(const pugi::xml_node &node) {

		NamedObject::set(node);

		properties.label = getAttribute(node,"label",properties.label);
		properties.summary = getAttribute(node,"summary",properties.summary);
		properties.url = getAttribute(node,"url",properties.url);
		properties.icon = getAttribute(node,"icon",properties.icon);

	}

	Value & Abstract::Object::getProperties(Value &value) const noexcept {
		return value;
	}

	Value & Object::getProperties(Value &value) const noexcept {

		NamedObject::getProperties(value);

		value["summary"] = summary();
		value["label"] = label();
		value["url"] = url();
		value["icon"] = icon();

		return value;
	}

	bool Object::getProperty(const char *key, std::string &value) const noexcept {

		if(NamedObject::getProperty(key,value)) {
			return true;
		}

		if(!strcasecmp(key,"label")) {
			value = properties.label;
		} else if(!strcasecmp(key,"summary")) {
			value = properties.summary;
		} else if(!strcasecmp(key,"url")) {
			value = properties.url;
		} else if(!strcasecmp(key,"icon")) {
			value = properties.icon;
		} else {
			return false;
		}
		return true;

	}

	bool NamedObject::getProperty(const char *key, std::string &value) const noexcept {
		if(!strcasecmp(key,"name")) {
			value = objectName;
			return true;
		}
		return false;
	}

	size_t NamedObject::hash() const noexcept {
		size_t seed = 0x19670123;
		size_t hash = 0;
		for(const char *ptr = objectName;*ptr;ptr++) {
			hash = (hash * seed) + ((size_t) *ptr);
		}
		return hash;
	}

	std::ostream & NamedObject::info() const {
		return std::cout << name() << "\t";
	}

	std::ostream & NamedObject::warning() const {
		return std::clog << name() << "\t";
	}

	std::ostream & NamedObject::error() const {
		return std::cerr << name() << "\t";
	}

	void Abstract::Object::for_each(const pugi::xml_node &root, const char *name, const char *group, const std::function<void(const pugi::xml_node &node)> &handler) {

		for(pugi::xml_node node = root.child(name); node; node = node.next_sibling(name)) {
			handler(node);
		}

		if(group && *group) {

			string group_name{root.name()};
			group_name += '-';
			group_name += group;

			string node_name{root.name()};
			node_name += '-';
			node_name += name;

			for(pugi::xml_node parent = root.parent(); parent; parent = parent.parent()) {

				// Scan for nodes.
				for(pugi::xml_node node = parent.child(node_name.c_str()); node; node = node.next_sibling(node_name.c_str())) {
					handler(node);
				}

				// Scan for groups.
				for(pugi::xml_node group = parent.child(group_name.c_str()); group; group = group.next_sibling(group_name.c_str())) {

					for(pugi::xml_node node = group.child(name); node; node = node.next_sibling(name)) {
						handler(node);
					}

				}

			}

		}

	}

	const char * Abstract::Object::settings_from(const XML::Node &node, bool upstream, const char *def) {

		auto attribute = node.attribute("settings-from");
		if(attribute) {
			return attribute.as_string(def);
		}

		string attrname{node.name()};
		attrname += "-defaults-from";
		attribute = node.attribute(attrname.c_str());
		if(attribute) {
			return attribute.as_string(def);
		}

		if(upstream) {
			for(XML::Node parent = node.parent(); parent; parent = parent.parent()) {
				attribute = parent.attribute(attrname.c_str());
				if(attribute) {
					return attribute.as_string(def);
				}
			}
		}

		if(*def) {
			return def;
		}

		return Quark( (string{node.name()} + "-defaults").c_str() ).c_str();
	}

	bool Abstract::Object::for_each(const pugi::xml_node &node, const char *tagname, const std::function<bool (const pugi::xml_node &node)> &call) {

		bool rc = false;

		for(pugi::xml_node n = node; n && !rc; n = n.parent()) {

			for(pugi::xml_node child = n.child(tagname); child && !rc; child = child.next_sibling(tagname)) {

				if(is_allowed(child)) {
					rc = call(child);
				}

			}

		}

		return rc;
	}

	const pugi::xml_attribute Abstract::Object::getAttribute(const pugi::xml_node &n, const char *name, bool change) {

		string key{name};
		pugi::xml_node node = n;

		while(node) {

			pugi::xml_attribute attribute = node.attribute(key.c_str());
			if(attribute) {
				return attribute;
			}

			for(auto child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

				if(strcasecmp(key.c_str(),child.attribute("name").as_string()) == 0) {
					return child.attribute("value");
				}

			}

			if(change) {
				change = false;
				key = node.name();
				key += "-";
				key += name;
			}

			node = node.parent();

		}

		return pugi::xml_attribute();

	}

	unsigned int Abstract::Object::getAttribute(const pugi::xml_node &node, const char *name, unsigned int def) {
		return getAttribute(node,name).as_uint(def);
	}

	unsigned int Abstract::Object::getAttribute(const pugi::xml_node &node, const char *group, const char *name, unsigned int def) {
		auto attribute = getAttribute(node,name);
		if(attribute) {
			return attribute.as_uint(def);
		}
		return Config::Value<unsigned int>(group,name,def);
	}

	bool Abstract::Object::getAttribute(const pugi::xml_node &node, const char *group, const char *name, bool def) {
		auto attribute = getAttribute(node,name);
		if(attribute) {
			return attribute.as_bool(def);
		}
		return Config::Value<bool>(group,name,def);
	}

	const char * Abstract::Object::getAttribute(const pugi::xml_node &node, const char *name, const char *def) {
		auto attribute = getAttribute(node,name);
		if(attribute) {
			return Quark(Udjat::expand(node,attribute,def)).c_str();
		}
		return def;
	}

	const char * Abstract::Object::getAttribute(const pugi::xml_node &node, const char *group, const char *name, const char *def) {

		auto attribute = getAttribute(node,name);
		if(attribute) {
			return Quark(Udjat::String(attribute.as_string(def)).expand(node)).c_str();
		}

		if(Config::hasKey(group,name)) {
			return Quark(Udjat::String(Config::get(group,name,def)).expand(node)).c_str();
		}

		return def;
	}

	std::string Abstract::Object::operator[](const char *key) const noexcept {
		string value;
		getProperty(key,value);
		return value;
	}

	string Abstract::Object::expand(const char *text, bool dynamic, bool cleanup) const {
		return String(text).expand([this](const char *key, std::string &value) {
			return getProperty(key,value);
		},dynamic,cleanup);
	}

	void Abstract::Object::expand(std::string &text, bool dynamic, bool cleanup) const {
		text = expand(text.c_str(),dynamic,cleanup);
	}

	const char * Abstract::Object::expand(const pugi::xml_node &node, const char *group, const char *value) {

		String text{value};
		text.expand([node,group](const char *key, string &value) {

			auto attribute = getAttribute(node,key);
			if(attribute) {
				value = Udjat::expand(node,attribute,"");
				return true;
			}

			if(Config::hasKey(group,key)) {
				value = Config::Value<string>(group,key,"");
				return true;
			}

			return false;

		});

		return Quark(text).c_str();

	}


 }
