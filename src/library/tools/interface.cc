/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implements the abstract interface for API Calls.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	static Container<Interface::Factory> & Factories() {
		static Container<Interface::Factory> instance;
		return instance;
	}

	void Interface::Factory::build(const XML::Node &node) noexcept {

		for(String &name : String{node,"type"}.split(",")) {

			for(auto &factory : Factories()) {

				if(*factory == name.c_str()) {

					try {

						factory->InterfaceFactory(node);

					} catch(const std::exception &e) {

						Logger::String{e.what()}.error(name.c_str());

					} catch(...) {

						Logger::String{"Unexpected error build interface"}.error(name.c_str());

					}

				}

			}

		}

	}

	Interface::Factory::Factory(const char *n) : _name {n} {
		Factories().push_back(this);
	}

	Interface::Factory::~Factory() {
		Factories().remove(this);
	}

	bool Interface::Factory::for_each(const std::function<bool(Interface::Factory &interface)> &method) {
		for(Interface::Factory *intf : Factories()) {
			if(method(*intf)) {
				return true;
			} 
		}
		return false;
	}

	Interface::Handler::Introspection::Introspection(const XML::Node &node) 
		: type{Value::TypeFactory(node)}, name{String{node,"name"}.as_quark()} {

		int dir = String{node,"direction","out"}.select("both","in","out",nullptr);
		if(dir < 0) {
			throw runtime_error("Invalid direction, should be both, in or out");
		}

		direction = (Direction) dir;

	}

	Interface::Handler::Handler(const char *name, const XML::Node &node) : _name{name} {
		for(XML::Node child = node.child("arg"); child; child = child.next_sibling("arg")) {
			introspection.emplace_back(child);
		}
	}

	Interface::Handler::Handler(const XML::Node &node) : Handler{String{node,"name"}.as_quark(),node} {
	}

	Interface::Handler::~Handler() {
	}

	void Interface::Handler::clear(Udjat::Value &request, Udjat::Value &response) const {
		request.clear(Value::Object);
		response.clear(Value::Object);
		for(auto val : introspection) {
			if(val.direction == Introspection::Input || val.direction == Introspection::Both) {
				request[val.name].clear(val.type);
			}
			if(val.direction == Introspection::Output || val.direction == Introspection::Both) {
				response[val.name].clear(val.type);
			}
		}
	}

	Interface::Interface(const XML::Node &node) {

		// Try type based name
		String attr{node.attribute("type").as_string("default"),"-name"};
		_name = String{node,attr.c_str()}.as_quark();
		if(_name && *_name) {
			return;
		}

		_name = String{node,"name"}.as_quark();
		if(!(_name && *_name)) {
			throw runtime_error("Required attribute 'name' is missing or empty");
		}

	}

	Interface::~Interface() {
	}

 }
