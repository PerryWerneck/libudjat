/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/agent.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/timestamp.h>

 using namespace std;

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

	bool Abstract::Agent::refresh(bool) {
		return false;
	}

	bool Abstract::Agent::updated(bool changed) noexcept {

		update.last = time(nullptr);

		if(update.timer && update.next <= update.last) {

			// Has timer, use it
			update.next = (update.last + update.timer);
			debug("Next update for '",name(),"' set to ",TimeStamp{update.next}.to_string().c_str());

		}

		if(!changed) {
			debug("Value of agent '",name(),"' not changed");
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

			if(!new_state->forward()) {

				// Not forward, Does any children has worst state?

				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children.agents) {
					if(child->level() > new_state->level()) {
						new_state = child->state();
					}
				}

			}

			set(new_state);

		} catch(const exception &e) {

			Logger::String{"Error '", e.what(), "' switching state"}.error(name());
			set(Abstract::State::Factory(e,"Error switching state"));

		} catch(...) {

			Logger::String{"Unexpected error switching state"}.error(name());
			set(make_shared<Abstract::State>("error",Udjat::critical,"Unexpected error switching state"));

		}

		return true;

	}

}
