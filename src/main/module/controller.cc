

#include "private.h"
#include <dlfcn.h>

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Module::Controller::guard;

	void start() noexcept {
		Module::getController().start();
	}

	void stop() noexcept {
		Module::getController().stop();
	}

	void reload() noexcept {
		Module::getController().reload();
	}

	Module::Controller & Module::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;
	}

	Module::Controller::Controller() {
	}

	Module::Controller::~Controller() {

		while(modules.size()) {

			Module * module = *modules.begin();
			Quark name = module->name;

			void *handle = module->handle;

			try {

				// First delete module
				delete module;

				// FIX-ME: Cant close module because of the protocol handlers.
				if(handle) {
					bool (*deinit)(void) = (bool (*)(void)) dlsym(handle,"udjat_module_deinit");
					auto err = dlerror();
					if(err) {
						cerr << err << endl;
					} else {

						if(!deinit()) {
							cout << name << "\tKeeping module open" << endl;
						} else if(dlclose(handle)) {
							cerr << name << "\tError '" << dlerror() << "' closing module" << endl;
						}

					}
				}


			} catch(const exception &e) {
				cerr << name << "\tError '" << e.what() << "' deinitializing module" << endl;
			} catch(...) {
				cerr << name << "\tUnexpected error deinitializing module" << endl;
			}

		}

	}

	void Module::Controller::insert(Module *module) {
		lock_guard<recursive_mutex> lock(guard);
		modules.push_back(module);
	}

	void Module::Controller::remove(Module *module) {
		lock_guard<recursive_mutex> lock(guard);

#ifdef DEBUG
		cout << "Removing module '" << module->name << "'" << endl;
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
			value["started"] = module->started;
			module->info->get(value);

			modules.append(value);
		}

		response["modules"] = modules;
	}


}

