
#include "private.h"

using namespace std;

namespace Udjat {

	Factory::Factory(const Quark &n) : name(n) {

		static const ModuleInfo info;
		this->info = &info;
		Controller::getInstance().insert(this);

	}

	Factory::~Factory() {
		Controller::getInstance().remove(this);
	}

	void Factory::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}

	bool Factory::parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) {
		return Controller::getInstance().parse(name,parent,node);
	}

}
