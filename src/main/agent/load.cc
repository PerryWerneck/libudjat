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

		bool upsearch = root.attribute("upsearch").as_bool(true);

		// Load my attributes
		struct Attr {
			/// @brief The attribute name.
			const char *name;
			const char **value;
		} attributes[] = {
			{ "summary",	&this->summary	},
			{ "label",		&this->label	},
			{ "icon",		&this->icon		},
			{ "uri",		&this->uri		}
		};

		for(size_t ix = 0; ix < (sizeof(attributes)/sizeof(attributes[0])); ix++) {
			const char * value = Quark().set(root,attributes[ix].name,false,translate).c_str();
			if(value && *value) {
				*attributes[ix].value = value;
			}
		}

		this->name = (Quark().set(root,"name",false)).c_str();

		this->update.timer = root.attribute("update-timer").as_uint(this->update.timer);
		this->update.on_demand = root.attribute("update-on-demand").as_bool(this->update.timer == 0);

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
