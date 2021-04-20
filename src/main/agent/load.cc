/**
 * @file
 *
 * @brief Implements the agent state machine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::load(const pugi::xml_node &root) {

		// Translate method
		auto translate = [root](const char *key) {
			return (const char *) root.attribute(key).as_string();
		};

		// Load my attributes
		this->name.set(root,"name",false);
		this->summary.set(root,"summary",false,translate);
		this->label.set(root,"label",false,translate);

//		this->update.notify = root.attribute("notify").as_bool(this->update.notify);
		this->update.timer = root.attribute("update-timer").as_uint(this->update.timer);
		this->update.on_demand = root.attribute("update-on-demand").as_bool(this->update.timer == 0);

		bool upsearch = root.attribute("upsearch").as_bool(true);
		this->icon.set(root,"icon",upsearch,translate);
		this->uri.set(root,"uri",upsearch,translate);

		time_t delay = root.attribute("delay-on-startup").as_uint(0);
		if(delay)
			this->update.next = time(nullptr) + delay;

		// Load children
		for(pugi::xml_node node : root) {

			// Skip reserved names.
			if(!strcasecmp(node.name(),"attribute")) {
				continue;
			}

			// Process factory methods.
			try {

				Factory::parse(node.name(), *this, node);

			} catch(const std::exception &e) {

				error("Error '{}' loading node '{}'",e.what(),node.name());

			} catch(...) {

				error("Unexpected error loading node '{}'",node.name());

			}

		}

	}

	void Abstract::Agent::load(const pugi::xml_document &doc) {

		for(pugi::xml_node root = doc.child("config"); root; root = root.next_sibling("config")) {
			load(root);
		}

	}


}
