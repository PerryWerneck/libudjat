

#include "private.h"

#ifndef _WIN32
	#include <dlfcn.h>
#endif // _WIN32

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Module::Controller::guard;

	Module::Controller & Module::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;
	}

	Module::Controller::Controller() {
		cout << "module\tStarting controller" << endl;
	}

	Module::Controller::~Controller() {

		cout << "module\tStopping controller" << endl;
		unload();

	}

	void Module::Controller::insert(Module *module) {
		lock_guard<recursive_mutex> lock(guard);
		modules.push_back(module);
	}

	void Module::Controller::remove(Module *module) {
		lock_guard<recursive_mutex> lock(guard);

#ifdef DEBUG
		cout << module->name << "\tRemoving module" << endl;
#endif // DEBUG

		modules.remove_if([module](Module *entry) {
			return entry == module;
		});
	}

	void Module::Controller::getInfo(Response &response) noexcept {

		response.reset(Value::Array);

		for(auto module : this->modules) {

			Value &object = response.append(Value::Object);

			object["name"] = module->name;
			module->info->get(object);

		}

	}


}

