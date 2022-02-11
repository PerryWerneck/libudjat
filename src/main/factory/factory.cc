
#include <udjat/defs.h>
#include "private.h"
#include <udjat/agent.h>
#include <udjat/state.h>
#include <udjat/moduleinfo.h>

using namespace std;

namespace Udjat {

	Factory::Factory(const char *n, const ModuleInfo &i) : factory_name(n), module(i) {
		Controller::getInstance().insert(this);
	}

	Factory::~Factory() {
		Controller::getInstance().remove(this);
	}

	void Factory::getInfo(Response &response) {
		response.reset(Value::Array);
		for_each([&response](const Factory &factory){
			factory.module.get(response.append(Value::Object))["name"] = factory.name();
			return false;
		});
	}

	bool Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		auto agent = AgentFactory(node);
		if(agent) {
			parent.insert(agent);
			return true;
		}
		auto object = ObjectFactory(parent,node);
		if(object) {
			parent.push_back(object);
			return true;
		}
		return false;
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	bool Factory::parse(Abstract::State &parent, const pugi::xml_node &node) const {
		return false;
	}
	#pragma GCC diagnostic pop

	bool Factory::for_each(std::function<bool(const Factory &factory)> func) {
		return Controller::getInstance().for_each(nullptr,func);
	}

	bool Factory::for_each(const char *name, std::function<bool(const Factory &factory)> func) {
		return Controller::getInstance().for_each(name,func);
	}

	std::shared_ptr<Abstract::Agent> Factory::AgentFactory(const pugi::xml_node UDJAT_UNUSED(&node)) const {
		return std::shared_ptr<Abstract::Agent>();
	}

	std::shared_ptr<Abstract::Object> Factory::ObjectFactory(const Abstract::Object UDJAT_UNUSED(&parent), const pugi::xml_node UDJAT_UNUSED(&node)) const {
		return std::shared_ptr<Abstract::Object>();
	}

	std::shared_ptr<Abstract::Alert> Factory::AlertFactory(const pugi::xml_node UDJAT_UNUSED(&node)) const {
		return std::shared_ptr<Abstract::Alert>();
	}

	std::ostream & Factory::info() const {
		return cout << name() << "\t";
	}

	std::ostream & Factory::warning() const {
		return clog << name() << "\t";
	}

	std::ostream & Factory::error() const {
		return cerr << name() << "\t";
	}

}
