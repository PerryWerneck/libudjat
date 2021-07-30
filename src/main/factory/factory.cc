
#include <udjat/defs.h>
#include "private.h"
#include <udjat/agent.h>
#include <udjat/state.h>

using namespace std;

namespace Udjat {

	static const ModuleInfo moduleinfo;

	Factory::Factory(const char *name) : Factory(name,&moduleinfo) {
	}

	Factory::Factory(const char *n, const ModuleInfo *i) : name(n), info(i) {
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
	std::shared_ptr<Abstract::Agent> Factory::factory(const char *id) const {
		throw system_error(ENOTSUP,system_category(),string{"Can't create agent '"} + this->name + "." + id + "'");
	}
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	bool Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		// throw runtime_error(string{"Element '"} + node.name() + "' is invalid at this context");
		return false;
	}
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	bool Factory::parse(Abstract::State &parent, const pugi::xml_node &node) const {
		// throw runtime_error(string{"Element '"} + node.name() + "' is invalid at this context");
		return false;
	}
	#pragma GCC diagnostic pop

	bool Factory::parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node) {
#ifdef DEBUG
		cout << __FUNCTION__ << ": " << name << endl;
#endif // DEBUG
		return Controller::getInstance().parse(name,parent,node);
	}

	bool Factory::parse(const char *name, Abstract::State &parent, const pugi::xml_node &node) {
		return Controller::getInstance().parse(name,parent,node);
	}

	std::shared_ptr<Abstract::Agent> Factory::get(const char *id) {

		const char * key = strchr(id,'.');

		if(!key) {
			throw system_error(EINVAL,system_category(),"Agent identifier should be in the format [FACTORY].id");
		}

		auto factory = Controller::getInstance().find(string(id,key-id).c_str());
		return factory->factory(key+1);

	}

}
