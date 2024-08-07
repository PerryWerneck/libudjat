/**
 * @file
 *
 * @brief Implements the agent update methods.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/alert.h>
 #include <udjat/tools/threadpool.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::updating(bool running) {

		if(running) {

			// Update is running.
			update.running = time(nullptr);

			if(update.timer) {
				update.next = (update.running + update.timer);
			} else {
				update.next = update.running + 10;
			}

		} else {

			// Update is complete.
			update.running = 0;
			updated(false);

		}

	}

	bool Abstract::Agent::chk4refresh(bool forward) {

		// Return if update is running.
		if(update.running)
			return false;

		if(!update.on_demand) {

			// It's not on-demand, check for timer and return if still waiting
			if(update.next && update.next > time(nullptr))
				return false;

		}

		updating(true);

		try {

			refresh(true);

		} catch(const std::exception &e) {

			updating(false);
			failed("Error updating agent",e);
			throw;

		} catch(...) {

			updating(false);
			failed("Unexpected error updating agent");
			throw;

		}

		if(forward) {
			// Check children
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children) {
				child->chk4refresh(true);
			}
		}

		updating(false);

		return true;

	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	bool Abstract::Agent::refresh(bool od) {
		return refresh();
	}
	#pragma GCC diagnostic pop

	bool Abstract::Agent::refresh() {
		return false;
	}

	bool Abstract::Agent::hasStates() const noexcept {
		return false;
	}

	bool Abstract::Agent::updated(bool changed) noexcept {

		update.last = time(nullptr);

		if(update.timer) {

			// Has timer, use it
			update.next = (update.last + update.timer);

		}

		if(!changed)
			return false;

		//
		// Value has changed, compute new state, emit alerts
		//
		bool level_has_changed = false;

		// Compute new state
		try {

			// First get state for current agent value.
			auto new_state = stateFromValue();

			// Does any children has worst state? if yes; use-it.
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children) {

					if(child->getLevel() > new_state->getLevel()) {
						new_state = child->getState();
					}
				}
			}

			level_has_changed = activate(new_state);

		} catch(const exception &e) {

			error("Error '{}' switching state",e.what());
			this->state.active = Abstract::State::get("Error switching state",e);
			this->state.activation = time(0);

		} catch(...) {

			error("Error '{}' switching state","unexpected");
			this->state.active = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");
			this->state.activation = time(0);

		}

		// Notify alerts.
		for(auto alert : alerts) {

			try {

				alert->set(*this,level_has_changed);

			} catch(const std::exception &e) {

				error("Error '{}' firing alert '{}'",e.what(),alert->c_str());

			} catch(...) {

				error("Unexpected error firing alert '{}'",alert->c_str());

			}

		}

		return true;

	}

}
