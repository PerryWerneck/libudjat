
#include "private.h"

using namespace std;

namespace Udjat {

	recursive_mutex Factory::Controller::guard;

	Factory::Controller::Controller() {

		insert(Atom::getFromStatic("integer"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<int>>(&parent,node);
		});

		insert(Atom::getFromStatic("int32"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<int32_t>>(&parent,node);
		});

		insert(Atom::getFromStatic("uint32"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<uint32_t>>(&parent,node);
		});

		insert(Atom::getFromStatic("boolean"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<bool>>(&parent,node);
		});

		insert(Atom::getFromStatic("string"),[](Abstract::Agent &parent, const pugi::xml_node &node){
				return make_shared<Udjat::Agent<std::string>>(&parent,node);
		});

		insert(Atom::getFromStatic("default"),[](Abstract::Agent &parent, const pugi::xml_node &node){
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
			"attribute"
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

	void Factory::Controller::load(std::shared_ptr<Abstract::Agent> parent, const pugi::xml_node &node) {

		// First load sub nodes
		for(auto child : node.children()) {

			//
			// Load agents.
			//
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
					auto agent = factory->second.create(*parent,child);
					this->load(agent,child);
					parent->children.push_back(agent);
				}
			} catch(const std::exception &e) {

				cerr << e.what() << endl;

			}

			//
			// Load other nodes.
			//
			{
				lock_guard<recursive_mutex> lock(guard);
				auto factory = nodes.find(name);
				if(factory != nodes.end()) {
					factory->second.apply(parent,child);
				}

			}

		}

		// And then, load states.
		for(pugi::xml_node state = node.child("state"); state; state = state.next_sibling("state")) {
			parent->append_state(state);
		}


	}

	void Factory::Controller::insert(const Atom &name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method) {
		lock_guard<recursive_mutex> lock(guard);
		Factory::validate_name(name.c_str());
		agents.insert(std::make_pair(name.c_str(),Agent(method)));
	}

	void Factory::Controller::insert(const Atom &name, std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> factory) {
		lock_guard<recursive_mutex> lock(guard);
		Factory::validate_name(name.c_str());
		nodes.insert(std::make_pair(name.c_str(),Node(factory)));
	}

	void Factory::insert(const Atom &name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method) {
		Factory::Controller::getInstance().insert(name,method);
	}

	void Factory::insert(const Atom &name, std::function<void(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_node &node)> factory) {
		Factory::Controller::getInstance().insert(name,factory);
	}

	void Factory::load(std::shared_ptr<Abstract::Agent> agent, const pugi::xml_document &doc) {

		auto factory = Factory::Controller::getInstance();

		for(pugi::xml_node node = doc.child("config"); node; node = node.next_sibling("config")) {
			factory.load(agent,node);
		}

	}

}
