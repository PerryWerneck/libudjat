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
 #include <udjat/tools/object.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/intl.h>
 #include <memory>
 
 using namespace std;

 namespace Udjat {
	
	Object::Object(const Udjat::Properties &props) : NamedObject{props} {
		properties.label = props["label"].as_quark(properties.label);
		properties.summary = props["summary"].as_quark(properties.summary);
		properties.url = props["url"].as_quark(properties.url);
		properties.icon = props["icon"].as_quark(properties.icon);
	}

	const char * Object::label() const noexcept {
		if(properties.label && *properties.label) {
			return properties.label;
		}
		return name();
	}

	const char * Object::icon() const noexcept {
		if(properties.icon && *properties.icon) {
			return properties.icon;
		}
		return "image-missing";
	}

	const char * Object::summary() const noexcept {
		return properties.summary;
	}
	
	Value & Object::get_properties(Value &value) const {

		NamedObject::get_properties(value);

		value["summary"] = summary();
		value["label"] = label();
		value["url"] = url();
		value["icon"].set(icon(),Value::Icon);

		return value;
	}

	bool Object::output_schema(const char *path, Schema &schema) const noexcept {

		NamedObject::output_schema(path,schema);

		schema.append(
			Schema::Item{ "summary",	Schema::String,	},
			Schema::Item{ "label",		Schema::String,	},
			Schema::Item{ "url",		Schema::String,	},
			Schema::Item{ "icon",		Schema::Icon,	}
		);

		return true;
	}

	bool Object::get_property(const char *key, Value &value) const {

		if(NamedObject::get_property(key,value)) {
			return true;
		} 

		if(!strcasecmp(key,"label")) {
			value = properties.label;
		} else if(!strcasecmp(key,"summary")) {
			value = summary();
		} else if(!strcasecmp(key,"url")) {
			value = properties.url;
		} else if(!strcasecmp(key,"icon")) {
			value = icon();
		} else {
			return false;
		}

		return true;

	}

	bool Object::get_property(const char *key, std::string &value) const {

		if(NamedObject::get_property(key,value)) {
			return true;
		} 

		if(!strcasecmp(key,"label")) {
			value = properties.label;
		} else if(!strcasecmp(key,"summary")) {
			value = summary();
		} else if(!strcasecmp(key,"url")) {
			value = properties.url;
		} else if(!strcasecmp(key,"icon")) {
			value = icon();
		} else {
			return false;
		}
		return true;

	}


 }
