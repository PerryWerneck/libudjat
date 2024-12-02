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

		int dir = String{node,"direction","out"}.select("none","in","out","both",nullptr);
		if(dir < 0) {
			throw runtime_error("Invalid direction, should be none, in, out or both");
		}

		switch(String{"node","value-from","none"}.select("none","path",nullptr)) {
		case 0:	// none
			break;

		case 1:	// From path
			dir |= FromPath;			
		}

		direction = (Direction) dir;


	}

	Interface::Handler::Handler(const char *name) : _name{name} {
	}

	Interface::Handler::Handler(const char *name, const XML::Node &node) : Handler{name} {
		for(XML::Node child = node.child("arg"); child; child = child.next_sibling("arg")) {
			introspection.emplace_back(child);
		}
	}

	Interface::Handler::Handler(const XML::Node &node) : Handler{String{node,"name"}.as_quark(),node} {
	}

	Interface::Handler::~Handler() {
	}

	bool Interface::Handler::for_each(const std::function<bool(const Introspection &instrospection)> &call) const {
		for(const auto &val : introspection) {
			if(call(val)) {
				return true;
			}
		}
		return false;
	}

	/*
	void Interface::Handler::clear(Udjat::Value &request, Udjat::Value &response) const {
		request.clear(Value::Object);
		response.clear(Value::Object);
		for(auto &val : introspection) {
			if(val.direction & Introspection::Input) {
				request[val.name].clear(val.type);
			}
			if(val.direction & Introspection::Output) {
				response[val.name].clear(val.type);
			}
		}
	}
	*/

	/*
	void Interface::Handler::prepare(Udjat::Request &request, Udjat::Response &response) const {


	}
	*/

	int Interface::Handler::call(Udjat::Request &request, Udjat::Response &response) const {

		//
		// Setup request/response
		//
		if(request != Value::Object) {
			request.clear(Value::Object);
		}

		if(response != Value::Object) {
			response.clear(Value::Object);
		}

		request.rewind();
		for(auto &val : introspection) {

			bool frompath = (val.direction & Introspection::FromPath);
			string value;
			if(frompath) {
				request.pop(value);
			}

			if( (val.direction & Introspection::Input) && (!request.contains(val.name) || frompath)) {

				// It's an input, update request.
				request[val.name].set(value.c_str(),val.type);

			}

			if( (val.direction & Introspection::Output) && (!response.contains(val.name) || frompath)) {

				// It's an output, update response.
				response[val.name].set(value.c_str(),val.type);

			}

		}

		//
		// Call actions
		//
		if(actions.empty()) {

			Logger::String{"Empty handler, just merging request into response"}.trace();
			response.merge(request);

		} else {

			for(auto action : actions) {
				request.rewind();
				int rc = action->call(request,response);
				if(rc) {
					Logger::String{"Action failed with rc=",rc}.trace(_name);
					return rc;
				}
			}
			
		}

		if(Logger::enabled(Logger::Debug)) {
			Logger::String{
				"Request:\n",request.serialize(MimeType::sh).c_str(),
				"\nResponse:\n",response.Udjat::Value::serialize(MimeType::sh).c_str()
			}.trace(_name);
		}

		return 0;
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
			throw runtime_error(Logger::String{"Required attribute 'name' or '",node.attribute("type").as_string("default"),"-name","' is missing or empty"});
		}

	}

	Interface::~Interface() {
	}

 }
