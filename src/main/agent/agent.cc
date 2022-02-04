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

/**
 *
 * @brief Implements the abstract agent methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::recursive_mutex Abstract::Agent::guard;

	Abstract::Agent::Agent(const char *name, const char *label, const char *summary) : Logger(name ? name : "") {

		state.active = Agent::stateFromValue();

		try {

			update.timer		= Config::get("agent-defaults","update-timer",update.timer);
			update.on_demand	= Config::get("agent-defaults","update-on-demand",update.timer == 0);
			update.next			= time(nullptr) + Config::get("agent-defaults","delay-on-startup",update.timer);
			update.failed 		= Config::get("agent-defaults","delay-when-failed",update.failed);

		} catch(const std::exception &e) {

			update.next	= time(nullptr) + update.timer;
			cerr << "Agent\tError '" << e.what() << "' loading defaults" << endl;

		}

		this->label = Quark((label ? label : name)).c_str();
		this->summary = Quark(summary).c_str();

	}

	Abstract::Agent::Agent(const pugi::xml_node UDJAT_UNUSED(&node)) : Abstract::Agent() {
	}

	Abstract::Agent::~Agent() {

		// Deleted! My children are now orphans.
		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {
			child->parent = nullptr;
		}

	}


	void Abstract::Agent::stop() {

#ifdef DEBUG
		cout << getName() << "\tStopping agent" << endl;
#endif // DEBUG

		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {

			try {

				child->stop();

			} catch(const exception &e) {

				child->failed("Agent stop has failed",e);

			}
		}
	}

	void Abstract::Agent::head(Response &response) {

		chk4refresh(true);

		time_t now = time(nullptr);

		// Gets the minor time for the next update.
		time_t next = now + Config::Value<time_t>("agent","max-update-time",600);

		// Gets the major time from the last update.
		time_t updated = 0;

		foreach([&next,&updated](Agent &agent){

			if(agent.update.next) {
				next = std::min(next,agent.update.next);
			}

			if(agent.update.last) {

				if(updated) {
					updated = std::max(updated,agent.update.last);
				} else {
					updated = agent.update.last;
				}
			}

		});

		if(next > now) {
			response.setExpirationTimestamp(next);
		} else {
			response.setExpirationTimestamp(0);
		}

		if(updated >= now) {
			response.setModificationTimestamp(updated);
		}

	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Abstract::Agent::append_state(const pugi::xml_node &node) {
		throw system_error(EPERM,system_category(),string{"Agent '"} + getName() + "' doesnt allow states");
	}
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Abstract::Agent::append_alert(const pugi::xml_node &node) {
		throw system_error(EPERM,system_category(),string{"Agent '"} + getName() + "' doesnt allow alerts");
	}
	#pragma GCC diagnostic pop

	std::shared_ptr<Abstract::State> Abstract::Agent::stateFromValue() const {

		static const Udjat::ModuleInfo moduleinfo {
			PACKAGE_NAME,									// The module name.
			"State factory",			 					// The module description.
			PACKAGE_VERSION, 								// The module version.
			PACKAGE_URL, 									// The package URL.
			PACKAGE_BUGREPORT 								// The bugreport address.
		};

		class DefaultState : public Abstract::State, Factory {
		public:

			DefaultState() : Abstract::State(""), Factory("state", &moduleinfo) {
			}

			~DefaultState() {
			}

			bool parse(Abstract::Agent &agent, const pugi::xml_node &node) const override {
				agent.append_state(node);
				return true;
			}

		};

		static shared_ptr<Abstract::State> state(new DefaultState());

		return state;

	}

}
