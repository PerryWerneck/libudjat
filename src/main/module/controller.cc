

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

	const Module * Module::Controller::find(const char *name) const noexcept {
		for(auto module : modules) {
			if(strcasecmp(module->name,name) == 0) {
				return module;
			}
		}
		return nullptr;
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
			module->info.get(object);

			void (*getInfo)(Value &object) = NULL;

			object["filename"] = module->filename();

#ifdef _WIN32
			getInfo = (void (*)(Value &object)) GetProcAddress(module->handle,"udjat_module_info");
#else
			{
				getInfo = (void (*)(Value &object)) dlsym(module->handle,"udjat_module_info");
				auto err = dlerror();
				if(err)
					getInfo = NULL;
			}
#endif // _WIN32

			if(getInfo) {
				getInfo(object);
			}

		}

	}

}

