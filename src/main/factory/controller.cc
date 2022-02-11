
#include "private.h"
#include <iostream>
#include <udjat/moduleinfo.h>

using namespace std;

namespace Udjat {

	recursive_mutex Factory::Controller::guard;

	Factory::Controller & Factory::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	/*
	void Factory::Controller::getInfo(Response &response) noexcept {

		response.reset(Value::Array);

		for(auto factory : factories) {

			Value &value  = response.append(Value::Object);

			value["name"] = factory->name();
			factory->module.get(value);

		}

	}
	*/

	void Factory::Controller::insert(const Factory *factory) {
		lock_guard<recursive_mutex> lock(guard);
		cout << "factories\tRegister '" << factory->name() << "' (" << factory->module.description << ")" << endl;
		factories.push_back(factory);
	}

	const Factory * Factory::Controller::find(const char *name) {
		lock_guard<recursive_mutex> lock(guard);

		for(auto factory : factories) {
			if(!strcasecmp(factory->name(),name)) {
				return factory;
			}
		}

		throw system_error(ENOENT,system_category(),string{"Cant find factory '"} + name + "'");
	}

	void Factory::Controller::remove(const Factory *factory) {

		cout << "factories\tUnregister '" << factory->name() << "' (" << factory->module.description << ")" << endl;

		lock_guard<recursive_mutex> lock(guard);
		factories.remove_if([factory](const Factory *obj){
			return factory == obj;
		});

	}

	/*
	bool Factory::Controller::parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) const {
		for(auto factory : factories) {
			if(!strcasecmp(factory->name(),name)) {
				factory->parse(parent,node);
				return true;
			}
		}
		return false;
	}
	*/

	/*
	bool Factory::Controller::parse(const char *name, Abstract::State &parent, const pugi::xml_node &node) const {
		for(auto factory : factories) {
			if(!strcasecmp(factory->name,name)) {
				factory->parse(parent,node);
				return true;
			}
		}
		return false;
	}
	*/

	bool Factory::Controller::for_each(const char *name, std::function<bool(const Factory &factory)> func) {
		lock_guard<recursive_mutex> lock(guard);
		for(auto factory : factories) {
			if(!(name && *name && strcasecmp(factory->name(),name))) {
				if(func(*factory)) {
					return true;
				}
			}
		}
		return false;
	}

}
