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
 #include <udjat/tools/object.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::recursive_mutex Abstract::Agent::guard;

	Abstract::Agent::Agent(const char *name, const char *label, const char *summary) : Object( (name && *name) ? name : "unnamed") {

		if(label && *label) {
			Object::properties.label = label;
		}

		if(summary && *summary) {
			Object::properties.summary = summary;
		}

		current_state.active = Agent::stateFromValue();

		try {

			update.timer		= Config::get("agent-defaults","update-timer",update.timer);
			update.on_demand	= Config::get("agent-defaults","update-on-demand",update.timer == 0);
			update.next			= time(nullptr) + Config::get("agent-defaults","delay-on-startup",update.timer);
			update.failed 		= Config::get("agent-defaults","delay-when-failed",update.failed);

		} catch(const std::exception &e) {

			update.next	= time(nullptr) + update.timer;
			cerr << "Agent\tError '" << e.what() << "' loading defaults" << endl;

		}

	}

	Abstract::Agent::Agent(const pugi::xml_node &node) : Abstract::Agent(Quark(node,"name","unnamed",false).c_str()) {
	}

	Abstract::Agent::~Agent() {

		// Deleted! My children are now orphans.
		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {
			child->parent = nullptr;
		}

	}


	void Abstract::Agent::stop() {

		lock_guard<std::recursive_mutex> lock(guard);

		for(auto childptr = children.rbegin(); childptr != children.rend(); childptr++) {

			auto agent = *childptr;
			try {

				agent->stop();

			} catch(const exception &e) {

				agent->error() << "Error '" << e.what() << "' while stopping" << endl;

			} catch(...) {

				agent->error() << "Unexpected error while stopping" << endl;

			}

		}

#ifdef DEBUG
		info() << "Stopping agent" << endl;
#endif // DEBUG

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
	std::shared_ptr<Abstract::State> Abstract::Agent::StateFactory(const pugi::xml_node &node) {
		throw system_error(EPERM,system_category(),string{"Agent '"} + name() + "' doesnt allow states");
	}
	#pragma GCC diagnostic pop

	void Abstract::Agent::push_back(std::shared_ptr<Abstract::Alert> UDJAT_UNUSED(alert)) {
		error() << "This agent is unable to handle alerts." << endl;
	}

	std::shared_ptr<Abstract::Alert> Abstract::Agent::AlertFactory(const pugi::xml_node &node) {

		const char *name = getAttribute(node,"alert-defaults","type","default");

		if(strcasecmp(name,"default")) {

			// It's not the default alert, search for factory.
			std::shared_ptr<Abstract::Alert> alert;
			Factory::for_each(name,[node,&alert](const Factory &factory){

				try {

					alert = factory.AlertFactory(node);

				} catch(const std::exception &e) {

					factory.error() << "Error '" << e.what() << "' creating alert" << endl;

				} catch(...) {

					factory.error() << "Unexpected error creating alert" << endl;

				}

				return (bool) alert;
			});

			return alert;

		}

		// Create default alert.
		return make_shared<Udjat::Alert>(node);

	}

	std::shared_ptr<Abstract::State> Abstract::Agent::stateFromValue() const {
		static shared_ptr<Abstract::State> instance;
		if(!instance) {
			cout << "states\tCreating default state" << endl;
			instance = make_shared<Abstract::State>("");
		}

		return instance;
	}

}
