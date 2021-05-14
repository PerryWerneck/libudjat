

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

	void Module::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}


}

