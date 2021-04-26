

#include "private.h"
#include <dlfcn.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const Quark &n) : started(false), name(n), handle(nullptr) {

		static const ModuleInfo info;
		this->info = &info;
		Controller::getInstance().insert(this);

	}

	Module::Controller & Module::getController() {
		return Controller::getInstance();
	}

	Module::~Module() {

		if(this->handle) {
			bool (*deinit)(void) = (bool (*)(void)) dlsym(handle,"udjat_module_deinit");
			auto err = dlerror();
			if(err) {
				cerr << err << endl;
			} else {
				deinit();
			}
		}

		Controller::getInstance().remove(this);
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

	void Module::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}


}

