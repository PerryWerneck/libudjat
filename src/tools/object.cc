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

 using namespace std;

 namespace Udjat {

	NamedObject::NamedObject(const pugi::xml_node &node) : NamedObject(Quark(node.attribute("name").as_string("unnamed")).c_str()) {
	}

	void NamedObject::set(const pugi::xml_node &node) {
		objectName = Quark(node.attribute("name").as_string(objectName)).c_str();
	}

	Value & NamedObject::getProperties(Value &value) const noexcept {
		value["name"] = objectName;
		return value;
	}

	Object::Object(const pugi::xml_node &node) : NamedObject(node) {
		set(node);
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

		value["name"] = name();
		value["summary"] = summary();
		value["label"] = label();
		value["url"] = url();
		value["icon"] = icon();

		return value;
	}

	bool Object::getProperty(const char *key, std::string &value) const noexcept {

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

	std::ostream & NamedObject::info() const {
		return std::cout << name() << "\t";
	}

	std::ostream & NamedObject::warning() const {
		return std::clog << name() << "\t";
	}

	std::ostream & NamedObject::error() const {
		return std::cerr << name() << "\t";
	}

	const char * Abstract::Object::getAttribute(const pugi::xml_node &node, const char *name, const char *def) {

		return Udjat::Attribute(
					node,
					name,
					(string(name) + "-" + node.name()).c_str()
				).c_str(def);

	}

	std::string Abstract::Object::operator[](const char *key) const noexcept {
		string value;
		getProperty(key,value);
		return value;
	}

	string Abstract::Object::expand(const char *text) const {
		return String(text).expand([this](const char *key, std::string &value) {
			return getProperty(key,value);
		});
	}


 }
