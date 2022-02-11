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
 #include <udjat/tools/object.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::load(const pugi::xml_node &root) {

		Object::set(root);

		const char *section = root.attribute("settings-from").as_string("agent-defaults");

		this->update.timer = getAttribute(root,section,"update-timer",(unsigned int) this->update.timer);
		this->update.on_demand = getAttribute(root,section,"update-on-demand",this->update.timer == 0);

		time_t delay = getAttribute(root,section,"delay-on-startup",(unsigned int) 0);
		if(delay)
			this->update.next = time(nullptr) + delay;

		// Load children
		for(pugi::xml_node node : root) {

			// Skip reserved names.
			if(strcasecmp(node.name(),"attribute") && strcasecmp(node.name(),"module")) {

				Factory::for_each(node.name(),[this,&node](const Factory &factory){

					try {

						return factory.parse(*this,node);

					} catch(const std::exception &e) {

						factory.error() << "Error '" << e.what() << "' parsing node <" << node.name() << ">" << endl;

					} catch(...) {

						factory.error() << "Unexpected error parsing node <" << node.name() << ">" << endl;

					}

					return false;

				});

			}


		}

	}

}
