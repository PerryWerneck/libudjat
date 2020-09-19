/**
 * @file src/core/agent/factory.cc
 *
 * @brief Implements the agent factory controller.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <cstring>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::recursive_mutex Abstract::Agent::Factory::guard;

	void set_factory_method(const char *name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> factory) {
		Abstract::Agent::Factory::getInstance().insert(name,factory);
	}

	Abstract::Agent::Factory & Abstract::Agent::Factory::getInstance() {
		static Agent::Factory instance;
		return instance;
	}

	Abstract::Agent::Factory::Factory() {

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

	Abstract::Agent::Factory::~Factory() {
	}

	void Abstract::Agent::Factory::insert(const char *name, std::function<std::shared_ptr<Abstract::Agent>(Abstract::Agent &parent, const pugi::xml_node &node)> method) {

		check_for_reserved_name(name);

		lock_guard<recursive_mutex> lock(guard);
		methods.insert(std::make_pair(Atom(name),Method(method)));

	}

	void Abstract::Agent::Factory::load(Abstract::Agent &parent,const pugi::xml_node &top) {

		for(auto node: top) {

			const char * name = node.name();
			if(!strcasecmp(name,"agent")) {
				name = node.attribute("type").as_string("default");
			}

			// Create agent.
			std::shared_ptr<Abstract::Agent> agent;
			{
				lock_guard<recursive_mutex> lock(guard);
				auto factory = methods.find(name);
				if(factory != methods.end())
					agent = factory->second.create(parent,node);

			}

			if(agent) {
				// Load children.
				agent->load(node);

				// And add it to the parent.
				parent.children.push_back(agent);
			}

		}

	}

}
