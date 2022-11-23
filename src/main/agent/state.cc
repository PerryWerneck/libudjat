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
				onStateChange(child_state,false,"State set to '{}' from child ({})");
			} else {
				onStateChange(state,true,"State restored to '{}' after child change ({})");
			}

		}

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

	bool Abstract::Agent::onStateChange(std::shared_ptr<State> state, bool activate, const char *message) {

		if(state.get() == this->current_state.active.get()) {
			debug("Changing to same state, ignored");
			return false;
		}

		auto saved_level = level();

		deactivate();

		this->current_state.active = state;

		if(activate) {
			this->activate();
		}

		notify(STATE_CHANGED);

		if(saved_level != this->level()) {

			if(message && *message) {

				std::string body{state->to_string()}; // Why is this necessary?

				LogFactory(this->level())
					<< name()
					<< "\t"
					<< Logger::Message{
							message,
							body,
							this->level()
						}
					<< endl;

			}

			notify(LEVEL_CHANGED);
		}
#ifdef DEBUG
		else {
			warning() << "State level stays the same, no message" << endl;
		}
#endif // DEBUG

		return true;
	}

	bool Abstract::Agent::set(std::shared_ptr<State> state) {

		// It's an empty state?.
		if(!state) {
			throw runtime_error("Cant set an empty state");
		}

		if(!onStateChange(state,true,"Current state changed to '{}' ({})")) {
			return false;
		}

		if(this->current_state.active->forwardToChildren()) {

			debug("Forwarding active state to children");

			for_each([this,state](Abstract::Agent &agent){

				if(agent.update.timer && agent.onStateChange(state,false,"State set to '{}' from parent ({})")) {
					agent.current_state.activated = false;
					agent.update.next = (max(this->update.next,time(0)) + agent.update.timer);
				}

			});

		}

		if(parent) {
			parent->onChildStateChange();
		}

		return true;

	}

}
