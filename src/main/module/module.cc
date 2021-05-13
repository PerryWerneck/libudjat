

#include "private.h"
#include <dlfcn.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const Quark &n) : started(false), name(n), handle(nullptr) {

		static const ModuleInfo info;
		this->info = &info;
		Controller::getInstance().insert(this);

	}

	Module::~Module() {
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

