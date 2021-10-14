

#include "private.h"

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const char *n, const ModuleInfo *i) : name(n),handle(nullptr),info(i) {
		if(!(info && name)) {
			throw system_error(EINVAL,system_category(),"Module info and name cant be null");
		}
		Controller::getInstance().insert(this);
	}

	Module::~Module() {
		Controller::getInstance().remove(this);
	}

	void Module::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}


}

