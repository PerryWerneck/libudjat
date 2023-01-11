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
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <udjat/alert/activation.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	/// @brief Activate an error state.
	void Abstract::Agent::failed(const char *summary, const std::exception &e) noexcept {

		error() << summary << ": " << e.what() << endl;

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		set(Udjat::StateFactory(e,summary));

	}

	void Abstract::Agent::failed(const char *summary, int code) noexcept {

		cerr << name() << "\t" << summary << ": " << strerror(code) << endl;

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		set(make_shared<Abstract::State>("error",Udjat::critical,summary,strerror(errno)));

	}

	/// @brief Set failed state from known exception
	void Abstract::Agent::failed(const char *summary, const char *body) noexcept {

		cerr << name() << "\t" << summary << endl;

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		set(make_shared<Abstract::State>("error",Udjat::critical,summary,body));

	}

	void Abstract::Agent::activate(std::shared_ptr<Abstract::Alert> alert) const {
		auto activation = alert->ActivationFactory();
		activation->set(*this);
		Udjat::start(activation);
	}

	std::ostream & LogFactory(Udjat::Level level) {
		if(level >= Udjat::error) {
			return cerr;
		} else if(level >= Udjat::warning) {
			return clog;
		}
		return cout;
	}

	bool Abstract::Agent::activate(std::shared_ptr<State> state) {
		return set(state);
	}

	bool Abstract::Agent::onStateChange(std::shared_ptr<State> state, bool activate, const char *message) {

		if(state.get() == this->current_state.selected.get()) {
			// It's the same state, just return.
			debug("Changing '",name(),"' to same state (",state->summary(),"), ignored");
			return false;
		}

		// Save current values.
		auto saved_level = level();
		auto saved_ready = ready();
		auto level = state->level();

		if(current_state.activation == current_state.Activation::StateWasActivated) {

			// Current state was activated, deactivate it

			debug("Agent '",name(),"' is deactivating state '",this->state()->summary(),"'");

			try {

				current_state.selected->deactivate();

			} catch(const std::exception &e) {

				error() << "Error '" << e.what() << "' deactivating state" << endl;

			} catch(...) {

				error() << "Unexpected error deactivating state" << endl;

			}

		}

		if(current_state.selected->forward()) {

			// Current state was forwarded? Check agent children and remove it from them.

			debug("Forwarding inactive state '",this->current_state.selected->summary(),"' from '",name(),"' to children");

			for_each([this](Abstract::Agent &agent){

				if(&agent != this && agent.current_state.selected.get() == this->current_state.selected.get()) {

					// Child is in the forwarded state, change it to default.

					agent.current_state.set(agent.computeState());
					debug("Removing forwarded state from agent '",agent.name(),"', new state is '",agent.current_state.selected->summary(),"'");
					if(agent.update.timer) {
						agent.update.next = time(0) + agent.update.timer;
					}
				}

			});

		}

		if(activate) {

			// Activate new state.

			try {

				current_state.activate(state);
				debug("Agent '",name(),"' is activating state '",this->state()->to_string().c_str(),"'");
				this->state()->activate(*this);

			} catch(const std::exception &e) {

				error() << "Error '" << e.what() << "' activating state" << endl;
				current_state.set(Udjat::StateFactory(e,_("Error activating state")));

			} catch(...) {

				error() << "Unexpected error activating state" << endl;
				current_state.set(make_shared<Abstract::State>("error",Udjat::critical,_("Unexpected error activating state")));

			}

		} else {

			// Dont activate, just set the new state.

			current_state.set(state);

		}

		debug("New state on '",name(),"' is '",this->state()->summary(),"'");

		// Notify listeners.
		notify(STATE_CHANGED);

		if(saved_level != level) {

			// State level has changed, log and notify.

			if(message && *message) {

				LogFactory(level)
					<< name()
					<< "\t"
					<< Logger::Message{
							message,
							this->state()->to_string(),
							level
						}
					<< endl;

			}

			notify(LEVEL_CHANGED);

			bool rd = this->ready();
			if(rd != saved_ready) {
				notify(rd ? READY : NOT_READY);
			}

		}
#ifdef DEBUG
		else {
			debug("State on  '",name(),"' is now '",state->summary(),"' with same level, no message");
		}
#endif // DEBUG

		return true;
	}

	bool Abstract::Agent::set(std::shared_ptr<State> state) {

		// It's an empty state?.
		if(!state) {
			throw runtime_error("Cant set an empty state");
		}

		if(parent && parent->current_state.activated() && parent->current_state.selected->forward()) {
			info() << "Ignoring state '" << state->summary() << "' by parent (" << parent->name() << ") request" << endl;
			return false;
		}

		if(onStateChange(state,true,"Current state changed to '{}' ({})")) {

			if(this->current_state.selected->forward()) {

				debug("Forwarding active state '",this->current_state.selected->summary(),"' from '",name(),"' to children");
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children.agents) {
					child->forward(this->current_state.selected);
				}

			}

			this->current_state.selected->refresh();

			// Update parent
			for(auto parent = this->parent; parent; parent = parent->parent) {

				auto state = parent->computeState();
				{
					lock_guard<std::recursive_mutex> lock(guard);
					for(auto child : parent->children.agents) {
						if(child->level() > state->level()) {
							state = child->state();
						}
					}
				}

				debug("Computed state for '",parent->name(),"' is ",state->summary()," (",state->level(),")");

				if(!parent->onStateChange(state,false,"Current state changed to '{}' by child request ({})")) {
					debug("No state change on '",parent->name(),"' stop update");
					break;
				}

			}

			return true;

		}

		return false;

	}

}
