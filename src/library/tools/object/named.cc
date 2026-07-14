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
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/value.h>

 using namespace std;

 namespace Udjat {

	NamedObject::NamedObject(const char *name, const Properties &props) : NamedObject{props.get("name",name).as_quark()} {
	}

	NamedObject::NamedObject(const Properties &props) : NamedObject{props["name"].as_quark()} {
	}

	const char * NamedObject::name() const noexcept {
		return objectName;
	}

	const char * NamedObject::c_str() const noexcept {
		return (this->objectName ? this->objectName : "" );
	}

	size_t NamedObject::push(std::function<void()> callback) {
		return ThreadPool::getInstance().push(objectName,callback);
	}

	int NamedObject::compare(const NamedObject &object ) const {
		return strcasecmp(this->c_str(),object.c_str());
	}

	bool NamedObject::operator==(const char *name) const noexcept {
		return strcasecmp(c_str(), name) == 0;
	}

	bool NamedObject::operator==(const Properties &node) const noexcept {
		return strcasecmp(c_str(),node["name"].c_str()) == 0;
	}

	Value & NamedObject::get_properties(Value &value) const {
		value["name"] = objectName;
		return value;
	}

	std::string NamedObject::to_string() const noexcept {
		return c_str();
	}

	bool NamedObject::get_property(const char *key, std::string &value) const {
		if(!strcasecmp(key,"name")) {
			value = objectName;
			return true;
		}
		return Abstract::Object::get_property(key,value);
	}

 }
