
#include "private.h"
#include <iostream>

using namespace std;

namespace Udjat {

	recursive_mutex Factory::Controller::guard;

	Factory::Controller & Factory::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	void Factory::Controller::getInfo(Response &response) noexcept {

		response.reset(Value::Array);

		for(auto factory : factories) {

			Value &value  = response.append(Value::Object);

			value["name"] = factory.second->name;
			factory.second->info->get(value);

		}

	}

	void Factory::Controller::insert(const Factory *factory) {
		lock_guard<recursive_mutex> lock(guard);

		cout << factory->name << "\tFactory registered" << endl;

		factories.insert(make_pair(factory->name,factory));

	}


	const Factory * Factory::Controller::find(const char *name) {
		lock_guard<recursive_mutex> lock(guard);

		auto entry = factories.find(name);
		if(entry == factories.end()) {
			throw system_error(ENOENT,system_category(),string{"Cant find factory '"} + name + "'");
		}

		return entry->second;
	}

	void Factory::Controller::remove(const Factory *factory) {
		lock_guard<recursive_mutex> lock(guard);

		cout << factory->name << "\tFactory unregistered" << endl;

		auto entry = factories.find(factory->name);
		if(entry == factories.end())
			return;

		if(entry->second != factory)
			return;

		factories.erase(entry);

	}

	bool Factory::Controller::parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) const {

		auto entry = factories.find(name);

		if(entry == factories.end()) {
#ifdef DEBUG
			cout << "Cant find factory for element '" << name << "'" << endl;
#endif // DEBUG
			return false;
		}

		entry->second->parse(parent,node);

		return true;
	}

	bool Factory::Controller::parse(const char *name, Abstract::State &parent, const pugi::xml_node &node) const {

		auto entry = factories.find(name);

		if(entry == factories.end()) {
#ifdef DEBUG
			cout << "Cant find factory for element '" << name << "'" << endl;
#endif // DEBUG
			return false;
		}

		entry->second->parse(parent,node);

		return true;
	}


}
