

#include "private.h"
#include <dlfcn.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const Quark &n, void *h) : started(false), name(n), handle(h) {

		static const ModuleInfo info = {

			"",	// The module name.
			"", // The module description.
			"", // The module version.
			"", // The bugreport address.
			"", // The package URL.

		};

		this->info = &info;

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

