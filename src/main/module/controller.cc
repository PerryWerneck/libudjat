

#include "private.h"

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
	}

	void Module::Controller::insert(Module *module) {
		lock_guard<recursive_mutex> lock(guard);
		modules.push_back(module);
	}

	void Module::Controller::remove(Module *module) {
		lock_guard<recursive_mutex> lock(guard);
		modules.remove_if([module](Module *entry) {
			return entry == module;
		});
	}

	void Module::Controller::start() noexcept {
		lock_guard<recursive_mutex> lock(guard);
		for(auto module : modules) {

			cout << module->name << "\tStarting" << endl;

			try {
				module->start();
			} catch (const exception &e) {
				cerr << module->name << "\tCan't start: " << e.what() << endl;
			}

		}
	}

	void Module::Controller::stop() noexcept {
		lock_guard<recursive_mutex> lock(guard);
		for(auto module : modules) {

			cout << module->name << "\tStopping" << endl;

			try {
				module->stop();
			} catch (const exception &e) {
				cerr << module->name << "\tCan't stop: " << e.what() << endl;
			}

		}
	}

	void Module::Controller::reload() noexcept {
		lock_guard<recursive_mutex> lock(guard);
		for(auto module : modules) {

			cout << module->name << "\tReloading" << endl;

			try {
				module->reload();
			} catch (const exception &e) {
				cerr << module->name << "\tCan't reload: " << e.what() << endl;
			}

		}
	}

}

