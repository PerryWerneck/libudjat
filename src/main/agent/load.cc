/**
 * @file
 *
 * @brief Implements the agent state machine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/module.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/alert/activation.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::setup_properties(const pugi::xml_node &root) noexcept {

		const char *section = root.attribute("settings-from").as_string("agent-defaults");

		this->update.timer = getAttribute(root,section,"update-timer",(unsigned int) this->update.timer);
		this->update.on_demand = getAttribute(root,section,"update-on-demand",this->update.timer == 0);

		time_t delay = getAttribute(root,section,"delay-on-startup",(unsigned int) (this->update.timer ? 1 : 0));
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
					sched_update(this->update.sigdelay);
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

	}

	void Abstract::Agent::setup_states(const pugi::xml_node &root) noexcept {

		// Load agent states.
		Abstract::Object::for_each(root,"state","states",[this](const pugi::xml_node &node){
			try {

				auto state = StateFactory(node);
				if(state) {
					state->setup(node);
				} else {
					error() << "Unable to create child state" << endl;
				}

			} catch(const std::exception &e) {

				error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

			} catch(...) {

				error() << "Unexpected error parsing <" << node.name() << ">" << endl;

			}
		});

	}

	void Abstract::Agent::setup_alerts(const pugi::xml_node &root) noexcept {

		// Load agent alerts.
		Abstract::Object::for_each(root,"alert","alerts",[this](const pugi::xml_node &node){

			// Create alerts.
			try {

				auto alert = AlertFactory(node);
				if(alert) {

					// Got alert, setup and insert it on agent.

					/// @brief Event listener, forward activity to alert.
					class Listener : public Abstract::Agent::EventListener {
					private:
						std::shared_ptr<Abstract::Alert> alert;

					public:
						Listener(const Abstract::Agent::Event e, std::shared_ptr<Abstract::Alert> a) : Abstract::Agent::EventListener{e}, alert{a} {
						}

						void trigger(const Event UDJAT_UNUSED(e), Abstract::Agent &agent) {

							auto activation = alert->ActivationFactory();

							activation->set(agent);
							activation->set(*agent.state());
							activation->set(agent.state()->level());

							Udjat::start(activation);
						}

					};

					alert->setup(node);

					switch(String{node,"trigger-event","default"}.select("state-change","level-change","value-change",NULL)) {
					case 0:	// on-state-change
						push_back(std::make_shared<Listener>(Event::STATE_CHANGED,alert));
						break;

					case 1:	// on-level-change
						push_back(std::make_shared<Listener>(Event::LEVEL_CHANGED,alert));
						break;

					case 2:	// on-value-change
						push_back(std::make_shared<Listener>(Event::VALUE_CHANGED,alert));
						break;

					default:
						debug("Using alert activation from agent '",name(),"'");
						push_back(node, alert);
					}

				} else {

					error() << "Unable to create alert" << endl;

				}

			} catch(const std::exception &e) {

				error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

			} catch(...) {

				error() << "Unexpected error parsing <" << node.name() << ">" << endl;

			}

		});
	}

	void Abstract::Agent::setup_children(const pugi::xml_node &root) noexcept {
		for(const pugi::xml_node &node : root) {

			// Check for validation.
			if(!is_allowed(node)) {
				continue;
			}

			// Create child.
			if(strcasecmp(node.name(),"module") == 0) {

				Module::load(node);

			} else if(strcasecmp(node.name(),"attribute")) {

				// It's not an attribute, check if it's a child node.
				ChildFactory(node);

			}

		}
	}

	void Abstract::Agent::setup(const pugi::xml_node &root) {

		setup_properties(root);
		setup_states(root);
		setup_alerts(root);
		setup_children(root);

		// Search for common states & alerts.
		string nodename{root.name()};
		nodename += "-defaults";
		for(XML::Node node = root.parent(); node; node = node.parent()) {
			for(auto child = node.child(nodename.c_str()); child; child = child.next_sibling(nodename.c_str())) {
				setup_states(child);
				setup_alerts(child);
			}
		}

	}

}
