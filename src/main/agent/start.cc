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
 #include <udjat/tools/logger.h>

 namespace Udjat {

	void Abstract::Agent::start() {

		debug("Starting agent '",name(),"' with value ",to_string().c_str());

		// Start children
		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children.agents) {

				try {

					if(!child->current_state.selected) {
						child->current_state.set(Abstract::Agent::computeState());
					}

					child->start();

				} catch(const std::exception &e) {

					child->failed("Agent startup has failed",e);

				}
			}
		}

		debug("Agent '",name(), "' started, updating state");

		// Update agent state.
		{
			auto first_state = computeState();

			if(!first_state) {
				warning() << "Got an invalid state, switching to the default one" << endl;
				first_state = Abstract::Agent::computeState();
			}


			// Check for children state
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children.agents) {
					if(child->level() > first_state->level()) {
						first_state = child->current_state.selected;
					}
				}
			}

			current_state.set(first_state);

			const char * name = this->current_state.selected->name();
			auto level = this->level();
			if(name && *name && level != Udjat::unimportant) {

				string value = to_string();

				if(value.empty()) {

					info()	<< "Starts with state '"
							<< this->current_state.selected->name()
							<< "' and level '"
							<< level
							<< "'"
							<< endl;

				} else {

					info()	<< "Starts with value '"
							<< value
							<< "', state '"
							<< this->current_state.selected->name()
							<< "' and level '"
							<< level
							<< "'"
							<< endl;

				}

			}

		}

		notify(STARTED);
		debug("Agent '",name(), "' start complete");

	}

 }
