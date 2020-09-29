
#include "private.h"

using namespace std;

namespace Udjat {

	recursive_mutex Factory::Controller::guard;

	Factory::Controller::Controller() {

		insert("integer",[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<int>>(&parent,node);
		});

		insert("int32",[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<int32_t>>(&parent,node);
		});

		insert("uint32",[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<uint32_t>>(&parent,node);
		});

		insert("boolean",[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<bool>>(&parent,node);
		});

		insert("string",[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<std::string>>(&parent,node);
		});

		insert("default",[](Abstract::Agent &parent, const pugi::xml_node &node){
//				return make_shared<Abstract::Agent>(&parent,node);
				return make_shared<Udjat::Agent<uint32_t>>(&parent,node);
		});

	}

	Factory::Controller::~Controller() {
	}

	Factory::Controller & Factory::Controller::getInstance() {

		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;

	}

	const char * Factory::validate_name(const char *name) {

		if(!(name && *name))
			throw runtime_error("Can't use empty name");

		// Check for reserved names.
		static const char *reserved_names[] = {
			"state",
			"agent",
			"action"
		};

		for(size_t ix = 0; ix < (sizeof(reserved_names)/sizeof(reserved_names[0])); ix++) {
			if(!strcasecmp(name,reserved_names[ix])) {
				string message("Can't use reserved name \'");
				message += name;
				message += "\'";
				throw std::runtime_error(message);
			}
		}

		return name;

	}

	void Factory::Controller::load(Abstract::Agent &parent, const pugi::xml_node &node) {

		// First load children.
		for(auto child : node.children()) {

			const char *name = child.name();
			if(strcasecmp(name,"agent") == 0) {
				name = child.attribute("type").as_string();
			}

			if(!name && *name)
				continue;

			try {
				lock_guard<recursive_mutex> lock(guard);
				auto factory = agents.find(name);
				if(factory != agents.end()) {
					auto agent = factory->second.create(parent,child);
					this->load(*agent,child);
					parent.children.push_back(agent);
				}
			} catch(const std::exception &e) {

				cerr << e.what() << endl;

			}

		}

		// Then load other elements.

	}

	void Factory::Controller::insert(const char *name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method) {
		lock_guard<recursive_mutex> lock(guard);
		agents.insert(std::make_pair(Atom(name),Agent(method)));
	}

	void Factory::insert(const char *name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method) {
		Factory::validate_name(name);
		Factory::Controller::getInstance().insert(name,method);
	}


}
