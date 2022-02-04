/**
 * @file
 *
 * @brief Implements the agent state machine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/module.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::load(const pugi::xml_node &root, bool name) {

		// Translate method
		auto translate = [root](const char *key) {
			return (const char *) root.attribute(key).as_string();
		};

		Object::set(root);

		this->update.timer = root.attribute("update-timer").as_uint(this->update.timer);
		this->update.on_demand = root.attribute("update-on-demand").as_bool(this->update.timer == 0);

		time_t delay = root.attribute("delay-on-startup").as_uint(0);
		if(delay)
			this->update.next = time(nullptr) + delay;

		// Load children
		for(pugi::xml_node node : root) {

			// Skip reserved names.
			if(!(strcasecmp(node.name(),"attribute") && strcasecmp(node.name(),"module"))) {
				continue;
			}

			// Process factory methods.
			try {

				Factory::parse(node.name(), *this, node);

			} catch(const std::exception &e) {

				cerr << Object::name() << "Error '" << e.what() << "' loading node '" << node.name() << "'" << endl;

			} catch(...) {

				cerr << Object::name() << "Unexpected error loading node '" << node.name() << "'" << endl;

			}

		}

	}

	/*
	void Abstract::Agent::load(const pugi::xml_document &doc) {

		for(pugi::xml_node root = doc.child("config"); root; root = root.next_sibling("config")) {

			const char *path = root.attribute("root-path").as_string();

			if(path && *path) {

				// Has defined root path, find agent.
				Abstract::Agent * agent = this;
				while(agent->parent) {
					agent = agent->parent;
				}

				agent->find(path,true,true)->load(root);

			} else {

				// No path, load here.
				load(root);

			}

		}

	}
	*/

}
