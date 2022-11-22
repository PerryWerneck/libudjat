/**
 * @file
 *
 * @brief Implements the agent update methods.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <private/agent.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::chk4refresh(bool forward) noexcept {

		lock_guard<std::recursive_mutex> lock(guard);

		// Return if update is running.
		if(update.running)
			return;

		if(update.on_demand) {

			// It's on demand, run agent update.

			update.running = time(0);

			try {

				refresh(true);

			} catch(const std::exception &e) {

				failed("Error updating agent",e);

			} catch(...) {

				failed("Unexpected error while updating");

			}


		}

		if(forward) {
			// Check children
			for(auto child : children.agents) {
				child->chk4refresh(true);
			}
		}

		update.running = 0;

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

	bool Abstract::Agent::updated(bool changed) noexcept {

		update.last = time(nullptr);

		if(update.timer) {

			// Has timer, use it
			update.next = (update.last + update.timer);

		}

		if(!changed) {
			notify(VALUE_NOT_CHANGED);
			return false;
		}

		try {

			//
			// Notify new value
			//
			debug("Agent '",name(),"' changed value to ",to_string().c_str());
			notify(VALUE_CHANGED);

			//
			// Compute new state
			//

			// First get state for current agent value.
			auto new_state = computeState();

			// Does any children has worst state? if yes; use-it.
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children.agents) {

					if(child->level() > new_state->level()) {
						new_state = child->state();
					}
				}
			}

			set(new_state);

		} catch(const exception &e) {

			error() << "Error '" << e.what() << "' switching state" << endl;
			set(Udjat::StateFactory(e,"Error switching state"));

		} catch(...) {

			cerr << name() << "\tUnexpected error switching state" << endl;
			this->current_state.active = make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state");

		}

		return true;

	}

}
