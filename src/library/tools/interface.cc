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
 #include <udjat/tools/properties.h>
 
 using namespace std;

 namespace Udjat {

	class UDJAT_PRIVATE InterfaceFactories : public Container<Interface::Factory>, public Properties::ObjectBuilder {
	public:
		InterfaceFactories() : Properties::ObjectBuilder{"interface"} {
			debug("Interface factories initialized");
		}

		~InterfaceFactories() {
			debug("Interface factories destroyed");
		}

		bool build(const Properties &props) override {
			Interface::Factory::build(props);
			return true; // Node was parsed.
		}

	};

	static InterfaceFactories & Factories() {
		static InterfaceFactories instance;
		return instance;
	}

	void Interface::Factory::build(const Properties &props) noexcept {

		throw runtime_error("need refactor");

		// //
		// // If interface has the 'action-name' atribute will build one
		// // single action for all related interfaces.
		// //
		// auto action_name = props["action-name"];
		// std::shared_ptr<Action> action;
		// if(!action_name.empty()) {

		// 	try {

		// 		action = Action::Factory::build(props);

		// 	} catch(const std::exception &e) {

		// 		Logger::String{e.what()}.error(action_name.c_str());
		// 		return;

		// 	} catch(...) {

		// 		Logger::String{"Unexpected error building action"}.error(action_name.c_str());
		// 		return;

		// 	}
		// }

		// for(String &name : props["type"].split(",")) {

		// 	for(auto &factory : Factories()) {

		// 		if(strcmp(name.c_str(),"*") == 0 || strcasecmp(name.c_str(),"all") == 0 || *factory == name.c_str()) {

		// 			try {

		// 				Interface &intf = factory->InterfaceFactory(props);

		// 				if(action) {
		// 					intf.push_back(props,action);
		// 				}

		// 				// Insert handlers
		// 				for(auto hdl = props.child("handler"); hdl; hdl = hdl.next_sibling("handler")) {
		// 					auto &handler = intf.push_back(hdl);
		// 					for(const char *nodename : { "action", "script" }) {
		// 						for(auto act = hdl.child(nodename); hdl; hdl = hdl.next_sibling(nodename)) {
		// 							handler.push_back(act);
		// 						}
		// 					}
		// 				}

		// 			} catch(const std::exception &e) {

		// 				Logger::String{e.what()}.error(factory->name());

		// 			} catch(...) {

		// 				Logger::String{"Unexpected error building interface"}.error(factory->name());

		// 			}

		// 		}

		// 	}

		// }

	}

	bool Interface::push_back(const Properties &, std::shared_ptr<Action>) {
		throw logic_error("This interface is unable to handle actions");
	}

	Interface::Handler & Interface::push_back(const Properties &) {
		throw logic_error("This interface cant accept dynamic actions");
	}

	int Interface::call(Udjat::Request &request, Udjat::Response &response) const {
		Logger::String{"This interface is unable to process request"}.error(name());
		return ENOTSUP;
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

	void Interface::Factory::get_properties(Udjat::Value &value) const {
		value["name"] = name();
		value["description"] = description();
	}

	bool Interface::Handler::input_schema(Schema &schema) const noexcept {
		return false;
	}

	bool Interface::Handler::output_schema(Schema &schema) const noexcept{
		return false;
	}

	Interface::Handler::Handler(const char *name) : handler_name{name} {
	}

	Interface::Handler::Handler(const char *name, const Properties &) : handler_name{name} {
	}

	Interface::Handler::Handler(const Properties &props) : Handler{props["name"].as_quark(),props} {
	}

	Interface::Handler::~Handler() {
	}

	void Interface::Handler::push_back(std::shared_ptr<Action> action) {
		actions.push_back(action);
	}

	void Interface::Handler::push_back(const Properties &props) {
		push_back(Action::Factory::build(props));
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

		// Check input properties
		{
			Schema schema;
			if(input_schema(schema)) {
				for(const auto &item : schema) {
					if(!request.contains(item.name())) {
						throw runtime_error(Logger::String{"Required argument is missing: ",item.description()});
					}
				}
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
	
	Interface::Interface(const Properties &props) : role{Authentication::RoleFactory(props)} {

		// Try type based name
		String attr{props.get("type","default").c_str(),"-name"};
		interface_name = props[attr.c_str()].as_quark();
		if(interface_name && *interface_name) {
			return;
		}

		// Check names.
		for(const char *attrname : { "action-name", "name"}) {
			interface_name = props[attrname].as_quark();
			if(interface_name && *interface_name) {
				return;
			}
		}

		throw runtime_error(Logger::String{"Required attribute 'name' or '",props.get("type","default").c_str(),"-name","' is missing or empty"});

	}

	bool Interface::allow(const Authentication::Role role) const {
		return role >= this->role;
	}

	Interface::~Interface() {
		debug("Deleting interface '",name(),"'");	
	}

 }
