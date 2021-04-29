
#include "private.h"
#include <udjat/agent.h>
#include <udjat/state.h>

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

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		throw runtime_error(string{"Element '"} + node.name() + "' is invalid at this context");
	}
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Factory::parse(Abstract::State &parent, const pugi::xml_node &node) const {
		throw runtime_error(string{"Element '"} + node.name() + "' is invalid at this context");
	}
	#pragma GCC diagnostic pop

	bool Factory::parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) {
		return Controller::getInstance().parse(name,parent,node);
	}

	bool Factory::parse(const char *name, Abstract::State &parent, const pugi::xml_node &node) {
		return Controller::getInstance().parse(name,parent,node);
	}

}
