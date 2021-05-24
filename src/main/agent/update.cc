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
				update.next = update.expires = (update.running + update.timer);
			} else {
				update.next = 0;
				update.expires = update.running + 10;	// TODO: Make it configurable.
			}

		} else {

			// Update is complete.
			update.running = 0;
			updated(false);

		}

	}

	bool Abstract::Agent::chk4refresh(bool forward) {

		// Return if update is running.
		if(update.running || (update.expires && update.expires > time(nullptr)))
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

	void Abstract::Agent::refresh(bool od) {
		refresh();
	}

	void Abstract::Agent::refresh() {
	}

	bool Abstract::Agent::hasStates() const noexcept {
		return false;
	}

	bool Abstract::Agent::updated(bool changed) noexcept {

		update.last = time(nullptr);

		if(update.timer) {

			// Has timer, use it
			update.next = update.expires = (update.last + update.timer);

		} else if(update.next > update.last) {

			// No timer, but next is set and valid
			update.expires = update.next;

		} else {

			// No timer, no next update, use default expiration time.
			update.expires = update.running + 60;	// TODO: Make it configurable.
			update.next = 0;

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
			auto new_state = find_state();

			// Does any children has worst state? if yes; use-it.
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children) {

					if(child->state->getLevel() > new_state->getLevel()) {
						new_state = child->state;
					}
				}
			}

			level_has_changed = activate(new_state);

		} catch(const exception &e) {

			error("Error '{}' switching state",e.what());
			this->state = Abstract::State::get("Error switching state",e);

		} catch(...) {

			error("Error '{}' switching state","unexpected");
			this->state = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");

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
