

#include "private.h"
#include <dlfcn.h>

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Module::Controller::guard;

	void start() noexcept {
		Module::controller().start();
	}

	void stop() noexcept {
		Module::controller().stop();
	}

	void reload() noexcept {
		Module::controller().reload();
	}

	Module::Controller & Module::controller() {
		return Controller::getInstance();
	}

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

	void Module::Controller::start() noexcept {
		lock_guard<recursive_mutex> lock(guard);
		for(auto module : modules) {

			if(module->started)
				continue;

			cout << module->name << "\tStarting " << module->info->description << endl;

			try {
				module->start();
				module->started = true;
			} catch (const exception &e) {
				cerr << module->name << "\tError '" << e.what() << "' on start method" << endl;
			} catch(...) {
				cerr << module->name << "\tUnexpected error on start method" << endl;
			}

		}
	}

	void Module::Controller::stop() noexcept {
		lock_guard<recursive_mutex> lock(guard);
		for(auto module : modules) {

			if(!module->started)
				continue;

			cout << module-> name << "\tStopping " << module->info->description << endl;

			try {
				module->stop();
				module->started = false;
			} catch (const exception &e) {
				cerr << module->name << "\tError '" << e.what() << "' on stop method" << endl;
			} catch(...) {
				cerr << module->name << "\tUnexpected error on stop method" << endl;
			}

		}
	}

	void Module::Controller::reload() noexcept {
		lock_guard<recursive_mutex> lock(guard);
		for(auto module : modules) {

			cout << module->name << "\tReloading " << module->info->description << endl;

			try {
				module->reload();
			} catch (const exception &e) {
				cerr << module->name << "\tError '" << e.what() << "' on reload method" << endl;
			} catch(...) {
				cerr << module->name << "\tUnexpected error on reload method" << endl;
			}

		}
	}

	void Module::Controller::getInfo(Response &response) {

		Json::Value modules(Json::arrayValue);

		for(auto module : this->modules) {

			Json::Value value(Json::objectValue);

			value["name"] = module->name.c_str();
			value["active"] = module->started;
			module->info->get(value);
			modules.append(value);

		}

		response["modules"] = modules;
	}


}

