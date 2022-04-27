/**
 * @file
 *
 * @brief Implements the agent state machine.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	/// @brief Activate an error state.
	void Abstract::Agent::failed(const char *summary, const std::exception &e) noexcept {

		error() << summary << ": " << e.what() << endl;

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		activate(Udjat::StateFactory(e,summary));

	}

	void Abstract::Agent::failed(const char *summary, int code) noexcept {

		cerr << name() << "\t" << summary << ": " << strerror(code) << endl;

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		activate(make_shared<Abstract::State>("error",Udjat::critical,summary,strerror(errno)));

	}

	/// @brief Set failed state from known exception
	void Abstract::Agent::failed(const char *summary, const char *body) noexcept {

		cerr << name() << "\t" << summary << endl;

		if(update.failed) {
			this->update.next = time(nullptr) + update.failed;
		}

		activate(make_shared<Abstract::State>("error",Udjat::critical,summary,body));

	}

	void Abstract::Agent::onLevelChange() {
	}

	void Abstract::Agent::onChildStateChange() noexcept {

		try {

			// Compute my current state based on value.
			auto state = stateFromValue();

			// Then check the children.
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children.agents) {
					if(child->level() > state->level()) {
						state = child->state();
					}
				}
			}

			activate(state);

		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' switching state" << endl;
			this->current_state.active = Udjat::StateFactory(e,"Error switching state");
			this->current_state.activation = time(0);

		} catch(...) {

			cerr << name() << "\tUnexpected error switching state" << endl;
			this->current_state.active = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");
			this->current_state.activation = time(0);

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

		// It's an empty state?.
		if(!state) {
			throw runtime_error("Activating an empty state");
		}

		// Return if it's the same.
		if(state == this->current_state.active)
			return false;

		auto level = state->level();

		LogFactory(level)
			<< name()
			<< "\tCurrent state changes from'"
			<< this->current_state.active->to_string()
			<< "' to '"
			<< state->to_string()
			<< "' (" << level << ")"
			<< endl;

		Udjat::Level saved_level = this->level();

		try {

			this->current_state.active->deactivate(*this);
			this->current_state.active = state;
			this->current_state.activation = time(0);
			this->current_state.active->activate(*this);

			if(parent)
				parent->onChildStateChange();

		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' switching state" << endl;
			this->current_state.active = Udjat::StateFactory(e,"Error switching state");
			this->current_state.activation = time(0);

		} catch(...) {

			error() << "Unexpected error switching state" << endl;
			this->current_state.active = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");
			this->current_state.activation = time(0);

		}

		return (saved_level != this->level());
	}

}
