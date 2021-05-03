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

	void Abstract::Agent::updating() {

		update.running = time(nullptr);

		if(update.timer) {
			update.next = update.expires = (update.running + update.timer);
		} else {
			update.next = 0;
			update.expires = update.running + 10;	// TODO: Make it configurable.
		}

	}

	void Abstract::Agent::updated() {

		update.last = time(nullptr);

		if(update.timer) {
			update.next = update.last + update.timer;
		}

		update.running = 0;

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

		updating();

		try {

			refresh();
			updated();

		} catch(const std::exception &e) {

			updated();
			failed(e,"Error updating agent");
			throw;

		} catch(...) {

			updated();
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

		update.running = 0;

		return true;

	}

	void Abstract::Agent::refresh() {

	}

	bool Abstract::Agent::hasOwnStates() const noexcept {
		return false;
	}

	void Abstract::Agent::onValueChange() noexcept {

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

			failed(e,"Error switching state");

		} catch(...) {

			failed("Unexpected error switching state");

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

	}

}
