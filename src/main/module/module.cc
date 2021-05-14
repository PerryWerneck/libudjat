

#include "private.h"
#include <dlfcn.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	static const ModuleInfo moduleinfo;

	Module::Module(const Quark &n, const ModuleInfo *i) : started(false), name(n), handle(nullptr), info(i) {
		Controller::getInstance().insert(this);
	}

	Module::Module(const Quark &n) : Module(n,&moduleinfo) {
	}

	Module::~Module() {
		Controller::getInstance().remove(this);
	}

	void Module::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}


}

