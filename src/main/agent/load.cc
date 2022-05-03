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
 #include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::load(const pugi::xml_node &root) {

		Object::set(root);

#ifdef DEBUG
		info() << "*** Loading from xml" << endl;
#endif // DEBUG

		const char *section = root.attribute("settings-from").as_string("agent-defaults");

		this->update.timer = getAttribute(root,section,"update-timer",(unsigned int) this->update.timer);
		this->update.on_demand = getAttribute(root,section,"update-on-demand",this->update.timer == 0);

		time_t delay = getAttribute(root,section,"delay-on-startup",(unsigned int) 0);
		if(delay)
			this->update.next = time(nullptr) + delay;

		// Load children
		for(pugi::xml_node node : root) {

			if(!strcasecmp(node.name(),"state")) {

				// Create states.
				try {

					if(!StateFactory(node)) {
						error() << "Unable to create child state" << endl;
					}

				} catch(const std::exception &e) {

					error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

				} catch(...) {

					error() << "Unexpected error parsing <" << node.name() << ">" << endl;

				}

			} else if(!strcasecmp(node.name(),"alert")) {

				// Create alerts.
				try {

					auto alert = AlertFactory(node);
					if(alert) {
						push_back(alert);
					} else {
						error() << "Unable to create alert" << endl;
					}

				} catch(const std::exception &e) {

					error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

				} catch(...) {

					error() << "Unexpected error parsing <" << node.name() << ">" << endl;

				}

			} else if(strcasecmp(node.name(),"module") == 0) {

				// Only load module if 'preload' is not set.
				if(!Config::Value<bool>("modules","preload-from-xml",true)) {
#ifdef DEBUG
					cout << "*** Loading module '" << node.attribute("name").as_string() << "' from xml" << endl;
#endif // DEBUG
					Module::load(node);
				}

			} else if(strcasecmp(node.name(),"attribute")) {

				push_back(node);

			}


		}

		{
			lock_guard<std::recursive_mutex> lock(guard);
			Controller::getInstance().insert(this,root);
		}

	}

}
