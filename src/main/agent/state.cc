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

	void Abstract::Agent::onChildStateChange() noexcept {

		// Compute my current state based on value.
		std::shared_ptr<Abstract::State> child_state;

		// Then check the children.
		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children.agents) {
				if(!child_state || child->level() > child_state->level()) {
					child_state = child->state();
				}
			}
		}

		if(child_state) {

			// Has child state, check for update
			auto state = computeState();

			if(child_state->level() > state->level()) {
				state = child_state;
			}

			if(state.get() != current_state.active.get()) {

				// State was changed.
				deactivate();
				auto level = state->level();

				if(this->current_state.active->level() != level) {
					LogFactory(level)
						<< name()
						<< "\tState set to '"
						<< "' to '"
						<< state->to_string()
						<< "' from child (" << level << ")"
						<< endl;
				}

				current_state.active = state;

			}

		}

		/*
		try {

			// Compute my current state based on value.
			auto state = computeState();

			// Then check the children.
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children.agents) {
					if(child->level() > state->level()) {
						state = child->state();
					}
				}
			}

			set(state);

		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' switching state" << endl;
			this->current_state.active = Udjat::StateFactory(e,_("Error switching state"));
			this->current_state.activation = time(0);

		} catch(...) {

			cerr << name() << "\tUnexpected error switching state" << endl;
			this->current_state.active = make_shared<Abstract::State>("error",Udjat::critical,_("Unexpected error switching state"));
			this->current_state.activation = time(0);

		}
		*/

	}

	void Abstract::Agent::activate(std::shared_ptr<Abstract::Alert> alert) const {
		auto activation = alert->ActivationFactory();

		const char *description = summary();
		if(!(description && *description)) {
			description = state()->summary();
		}
		if(description && *description) {
			activation->set(description);
		}

		activation->set(*this);
		activation->set(state()->level());
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

	bool Abstract::Agent::set(std::shared_ptr<State> state) {

		// It's an empty state?.
		if(!state) {
			throw runtime_error("Cant set an empty state");
		}

		// Return if it's the same.
		if(state.get() == this->current_state.active.get())
			return false;

		auto level = state->level();

		if(this->current_state.active->level() != level) {
			LogFactory(level)
				<< name()
				<< "\tCurrent state changes from '"
				<< this->current_state.active->to_string()
				<< "' to '"
				<< state->to_string()
				<< "' (" << level << ")"
				<< endl;
		}

		Udjat::Level saved_level = this->level();

		deactivate();
		this->current_state.active = state;
		activate();

		if(this->current_state.active->forwardToChildren()) {

			debug("Forwarding state to children");

			for_each([this,state](Abstract::Agent &agent){

				if(agent.update.timer && agent.current_state.active != state) {
					agent.deactivate();
					agent.current_state.active = state;
					agent.info() << "State set to '" << agent.current_state.active->to_string() << "' (forwarded)" << endl;
					agent.update.next = (max(this->update.next,time(0)) + agent.update.timer);
				}

			});

		}

		notify(STATE_CHANGED);

		if(saved_level != this->level()) {
			notify(LEVEL_CHANGED);
		}

		if(parent) {
			parent->onChildStateChange();
		}

		return true;

	}

}
