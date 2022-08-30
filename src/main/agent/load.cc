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

	void Abstract::Agent::setup(const pugi::xml_node &root) {

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
					info() << signame << " (" << event.to_string() << ") will trigger an agent update" << endl;
				}

			} else {
				this->update.sigdelay = -1;
			}

		}
#endif // !_WIN32

		// Load agent states.
		Abstract::Object::for_each(root,"state","states",[this](const pugi::xml_node &node){
			try {

				auto state = StateFactory(node);
				if(!state) {
					error() << "Unable to create child state" << endl;
				}
				state->setup(node);

			} catch(const std::exception &e) {

				error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

			} catch(...) {

				error() << "Unexpected error parsing <" << node.name() << ">" << endl;

			}
		});

		// Load agent alerts.
		Abstract::Object::for_each(root,"alert","alerts",[this](const pugi::xml_node &node){

			// Create alerts.
			try {

				auto alert = AlertFactory(node);
				if(alert) {
					alert->setup(node);
					push_back(alert);
				} else {
					error() << "Unable to create alert" << endl;
				}

			} catch(const std::exception &e) {

				error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

			} catch(...) {

				error() << "Unexpected error parsing <" << node.name() << ">" << endl;

			}

		});

		for(const pugi::xml_node &node : root) {

			if(strcasecmp(node.name(),"module") == 0) {

				// Only load module if 'preload' is not set.
				if(!Config::Value<bool>("modules","preload-from-xml",true)) {
					Module::load(node);
				}

			} else if(strcasecmp(node.name(),"attribute")) {

				// It's not an attribute, check if it's a child node.
				ChildFactory(node);

			}

		}

	}

}
