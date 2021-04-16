

#include "private.h"
#include <dlfcn.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const Quark &n, void *h) : name(n), handle(h) {
		package.name 		= name.c_str();
		package.description	= "";
		package.version		= "";
		package.bugreport	= "";
		package.url			= "";
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

}

