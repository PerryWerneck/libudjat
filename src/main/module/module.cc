

#include "private.h"
#include <dlfcn.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const Quark &n, void *h) : name(n), handle(h) {
		Controller::getInstance().insert(this);
	}

	Module::Controller & Module::getController() {
		return Controller::getInstance();
	}

	Module::~Module() {
		Controller::getInstance().remove(this);
		if(handle)
			dlclose(handle);
	}

	Module * Module::load(const char *filename) {

		dlerror();

		void * handle = dlopen(filename,RTLD_NOW|RTLD_LOCAL);
		if(handle == NULL) {
			throw runtime_error(dlerror());
		}

		 Module * (*init)(void *);

		 Module * module = nullptr;
		 try {

			init = (Module * (*)(void *)) dlsym(handle,"udjat_module_init");
			auto err = dlerror();
			if(err)
				throw runtime_error(err);

			module = init(handle);
			if(!module) {
				throw runtime_error("Can't initialize module");
			}

			if(!module->handle) {
				clog << "Strange module initialization on '" << filename << "'" << endl;
				module->handle = handle;
			}

		 } catch(const exception &e) {

			dlclose(handle);
			handle = NULL;
			throw;

		 }

		 return module;
	}

	/// @brief Start module.
	void Module::start() {
	}

	/// @brief Reload module.
	void Module::reload() {
		stop();
		start();
	}

	/// @brief Stop module.
	void Module::stop() {
	}

}

