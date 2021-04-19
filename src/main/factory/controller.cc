
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

	void Factory::Controller::getInfo(Response &response) {

		Json::Value report(Json::arrayValue);

		for(auto method : methods) {

			Json::Value value(Json::objectValue);

			value["name"] = method.second->name.c_str();
			method.second->info->get(value);

			report.append(value);
		}

		response["factory"] = report;

	}

	void Factory::Controller::insert(const Factory *factory) {
		lock_guard<recursive_mutex> lock(guard);
#ifdef DEBUG
		cout << "Insering factory '" << factory->name << "'" << endl;
#endif // DEBUG

		methods.insert(make_pair(factory->name.c_str(),factory));

	}

	void Factory::Controller::remove(const Factory *factory) {
		lock_guard<recursive_mutex> lock(guard);

#ifdef DEBUG
		cout << "Removing factory '" << factory->name << "'" << endl;
#endif // DEBUG

		auto entry = methods.find(factory->name.c_str());
		if(entry == methods.end())
			return;

		if(entry->second != factory)
			return;

		methods.erase(entry);

	}

	bool Factory::Controller::parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) const {

		auto entry = methods.find(name);

		if(entry == methods.end()) {
#ifdef DEBUG
			cout << "Cant find factory for element '" << name << "'" << endl;
#endif // DEBUG
			return false;
		}

		entry->second->parse(parent,node);

		return true;
	}

}
