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

		//
		// If interface has the 'action-name' atribute will build one
		// single action for all related interfaces.
		//
		std::shared_ptr<Action> action;
		if(!String{node,"action-name"}.empty()) {

			try {

				action = Action::Factory::build(node,"action-name",true);

			} catch(const std::exception &e) {

				Logger::String{e.what()}.error(String{node,"action-name"}.c_str());
				return;

			} catch(...) {

				Logger::String{"Unexpected error building action"}.error(String{node,"action-name"}.c_str());
				return;

			}
		}

		for(String &name : String{node,"type"}.split(",")) {

			for(auto &factory : Factories()) {

				if(strcmp(name.c_str(),"*") == 0 || strcasecmp(name.c_str(),"all") == 0 || *factory == name.c_str()) {

					try {

						Interface &intf = factory->InterfaceFactory(node);

						if(action) {
							intf.push_back(node,action);
						}

						// Insert handlers
						for(auto hdl = node.child("handler"); hdl; hdl = hdl.next_sibling("handler")) {
							auto &handler = intf.push_back(hdl);
							for(const char *nodename : { "action", "script" }) {
								for(auto act = hdl.child(nodename); hdl; hdl = hdl.next_sibling(nodename)) {
									handler.push_back(act);
								}
							}
						}

					} catch(const std::exception &e) {

						Logger::String{e.what()}.error(factory->name());

					} catch(...) {

						Logger::String{"Unexpected error building interface"}.error(factory->name());

					}

				}

			}

		}

	}

	bool Interface::push_back(const XML::Node &, std::shared_ptr<Action>) {
		throw logic_error("This interface is unable to handle actions");
	}

	Interface::Factory::Factory(const char *name, const char *description) : factory_name{name}, factory_description{description} {
		Factories().push_back(this);
	}

	Interface::Factory::~Factory() {
		Factories().remove(this);
	}

	bool Interface::Factory::for_each(const std::function<bool(Interface::Factory &intf)> &method) {
		for(Interface::Factory *intf : Factories()) {
			if(method(*intf)) {
				return true;
			} 
		}
		return false;
	}

	void Interface::Factory::getProperties(Udjat::Value &value) const {
		value["name"] = name();
		value["description"] = description();
	}

	Interface::Handler::Introspection::Introspection(const XML::Node &node) 
		: type{Value::TypeFactory(node,"type")}, name{String{node,"name"}.as_quark()} {

		int dir = String{node,"direction","out"}.select("none","in","out","both",nullptr);
		if(dir < 0) {
			throw runtime_error("Invalid direction, should be none, in, out or both");
		}

		switch(String{node,"value-from","none"}.select("none","path",nullptr)) {
		case 0:	// none
			break;

		case 1:	// path
			dir |= FromPath;
			break;

		default:
			throw runtime_error(Logger::String{"Unexpected value '",String{node,"value-from"}.c_str(),"' on value-from attribute"});			
		}

		direction = (Direction) dir;


	}

	Interface::Handler::Handler(const char *name) : handler_name{name} {
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

	void Interface::Handler::push_back(std::shared_ptr<Action> action) {
		actions.push_back(action);
	}

	void Interface::Handler::push_back(const XML::Node &node) {
		push_back(Action::Factory::build(node,"name",true));
	}
	
	int Interface::Handler::call(Udjat::Request &request, Udjat::Response &response) const {

		//
		// Setup request/response
		//
		if(request != Value::Object) {
			debug("Cleaning request");
			request.clear(Value::Object);
		}

		if(response != Value::Object) {
			debug("Cleaning response");
			response.clear(Value::Object);
		}

		request.rewind();
		for(auto &val : introspection) {

			bool frompath = (val.direction & Introspection::FromPath);
			string value;
			if(frompath) {
				debug("Getting '",val.name,"' from path");
				request.pop(value);
			}

			if( (val.direction & Introspection::Input) && (!request.contains(val.name) || frompath)) {

				// It's an input, update request.
				request[val.name].set(value.c_str(),val.type);
				debug("request[",val.name,"]='",value.c_str(),"' '",std::to_string(request[val.name]).c_str(),"'");
			}

			if( (val.direction & Introspection::Output) && (!response.contains(val.name) || frompath)) {

				// It's an output, update response.
				response[val.name].set(value.c_str(),val.type);
				debug("response[",val.name,"]='",value.c_str(),"'");
			}

		}

		//
		// Call actions
		//
		if(Logger::enabled(Logger::Debug)) {
			Logger::String{
				"Handling '",request.path(),"'\n",
				"Request:\n",request.Udjat::Value::serialize(MimeType::yaml).c_str()
			}.trace(name());
		}

		if(actions.empty()) {

			Logger::String{"Empty handler, just merging request into response"}.trace(name());
			response.merge(request);

		} else {

			for(auto action : actions) {
				request.rewind();
				int rc = action->call(request,response);
				if(rc) {
					Logger::String{"Action failed with rc=",rc}.trace(name());
					return rc;
				}
			}
			
		}

		if(Logger::enabled(Logger::Debug)) {
			Logger::String{
				"Action suceedeed\nResponse:\n",response.Udjat::Value::serialize(MimeType::yaml).c_str()
			}.trace(name());
		}

		return 0;
	}

	Interface::Interface(const XML::Node &node) {

		// Try type based name
		String attr{node.attribute("type").as_string("default"),"-name"};
		interface_name = String{node,attr.c_str()}.as_quark();
		if(interface_name && *interface_name) {
			return;
		}

		// Check names.
		for(const char *attrname : { "name", "action-name"}) {
			interface_name = String{node,attrname}.as_quark();
			if(interface_name && *interface_name) {
				return;
			}
		}

		throw runtime_error(Logger::String{"Required attribute 'name' or '",node.attribute("type").as_string("default"),"-name","' is missing or empty"});

	}

	Interface::~Interface() {
		debug("Deleting interface '",name(),"'");	
	}

 }
