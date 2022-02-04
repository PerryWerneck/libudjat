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

		activate(StateFactory(e,summary));

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
				for(auto child : children) {
					if(child->level() > state->level()) {
						state = child->getState();
					}
				}
			}

			activate(state);

		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' switching state" << endl;
			this->state.active = StateFactory(e,"Error switching state");
			this->state.activation = time(0);

		} catch(...) {

			cerr << name() << "\tUnexpected error switching state" << endl;
			this->state.active = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");
			this->state.activation = time(0);

		}

	}

	void Abstract::Agent::activate(std::shared_ptr<Abstract::Alert> alert) const {
		Abstract::Alert::activate(alert,[this](std::string &text) {
			text = this->expand(text.c_str());
		});
	}

	bool Abstract::Agent::activate(std::shared_ptr<State> state) {

		// It's an empty state?.
		if(!state) {
			throw runtime_error("Activating an empty state");
		}

		// Return if it's the same.
		if(state == this->state.active)
			return false;

		string value = to_string();

		if(value.empty()) {
			info()	<< "Current state changes from'"
					<< this->state.active
					<< "' to '"
					<< state
					<< "(" << state->level() << ")"
					<< endl;

		} else {
			info()	<< "Value '" << value << "' changes state from '"
					<< this->state.active->summary()
					<< "' to '"
					<< state->summary()
					<< "(" << state->level() << ")"
					<< endl;
		}

		Udjat::Level saved_level = this->level();

		try {

			this->state.active->deactivate(*this);
			this->state.active = state;
			this->state.activation = time(0);
			this->state.active->activate(*this);

			if(parent)
				parent->onChildStateChange();

		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' switching state" << endl;
			this->state.active = StateFactory(e,"Error switching state");
			this->state.activation = time(0);

		} catch(...) {

			error() << "Unexpected error switching state" << endl;
			this->state.active = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");
			this->state.activation = time(0);

		}

		return (saved_level != this->level());
	}

}
