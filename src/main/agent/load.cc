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
 #include <udjat/tools/event.h>
 #include <udjat/tools/mainloop.h>

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

#ifndef _WIN32
		{
			// Check for signal based update.
			const char *signame = root.attribute("update-signal").as_string();
			if(*signame && strcasecmp(signame,"none")) {

				// Agent has signal based update.
				this->update.sigdelay = (short) getAttribute(root,section,"update-signal-delay",(unsigned int) 0);

				Udjat::Event &event = Udjat::Event::SignalHandler(this, signame, [this](){
					requestRefresh(this->update.sigdelay);
					return true;
				});

				if(this->update.sigdelay) {
					info()	<< "An agent update with a "
							<< this->update.sigdelay
							<< " second(s) delay will be triggered by signal '"
							<< event.to_string() << "'"
							<< endl;
				} else {
					info() << "Signal '" << event.to_string() << "' will trigger an agent update" << endl;
				}

			} else {
				this->update.sigdelay = -1;
			}

		}
#endif // !_WIN32

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

		/*
		{
			lock_guard<std::recursive_mutex> lock(guard);
			Controller::getInstance().insert(this,root);
		}
		*/

	}

}
