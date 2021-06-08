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

 #include "private.h"

 namespace Udjat {

	void Abstract::Agent::start() {

#ifdef DEBUG
		cout << getName() << "\tStarting agent" << endl;
#endif // DEBUG

		// Start children
		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children) {

				try {

					child->start();

				} catch(const std::exception &e) {

					child->failed("Agent startup has failed",e);

				}
			}
		}

		// Update agent state.
		{
			this->state.active = find_state();
			if(!this->state.active) {
				this->state.active = get_default_state();
			}

			// Check for children state
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children) {
					if(child->getLevel() > this->getLevel()) {
						this->state.active = child->state.active;
					}
				}
			}

			this->state.activation = time(0);

			const char * name = this->state.active->getName();
			if(name && *name) {
				info("Agent started on state '{}' ('{}')",
					this->state.active->getName(),
					std::to_string(this->getLevel())
				);
			}

		}

	}

 }
