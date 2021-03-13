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

		// Load my attributes
		this->name.set(root,"name");
		this->summary.set(root,"summary");
		this->label.set(root,"label");

		this->update.notify = root.attribute("notify").as_bool(this->update.notify);
		this->update.timer = root.attribute("update-timer").as_uint(this->update.timer);
		this->update.on_demand = root.attribute("update-on-demand").as_bool(this->update.timer == 0);

		bool upsearch = root.attribute("upsearch").as_bool(true);
		this->icon.set(root,"icon",upsearch);
		this->uri.set(root,"uri",upsearch);

		time_t delay = root.attribute("delay-on-startup").as_uint(0);
		if(delay)
			this->update.next = time(nullptr) + delay;

		// Load children
		for(pugi::xml_node node : root) {

			try {

				Abstract::Agent::Factory::parse(node.name(), *this, node);

			} catch(const std::exception &e) {

				cerr << "Erro loading node '" << node.name() << "': " << e.what() << endl;

			}

		}

	}

	void Abstract::Agent::load(const pugi::xml_document &doc) {

		auto agent = this;

		for(pugi::xml_node root = doc.child("config"); root; root = root.next_sibling("config")) {
			load(root);
		}

	}


}
