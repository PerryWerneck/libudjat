/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 * @file
 *
 * @brief Implements XML parser for Abstract::Agent.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/alert.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::parse(const XML::Node &node) {
		Controller::setup_properties(*this,node);
		Udjat::Object::parse(node);
	}

	bool Abstract::Agent::parse_child(const XML::Node &node) {

		if(Udjat::Object::parse_child(node)) {
			return true;
		}

		// It's a state?
		if(strcasecmp(node.name(),"state") == 0) {

			auto state = StateFactory(node);
			if(state) {
				for(XML::Node child : node) {
					if(is_reserved(node) || !is_allowed(node)) {
						continue;
					}
					state->parse_child(child);
				}
			} else {
				Logger::String{"Unable to create agent state"}.error(name());
			}

			return true;
		}

		// It's an agent?
		if(strcasecmp(node.name(),"agent") == 0) {

			auto agent = Abstract::Agent::Factory::build(*this,node);
			if(agent) {

				// TODO: Check agent-path attribute and change parent if needed.

				// Parse children, insert into child list.
				agent->parse(node);
				push_back(agent);

				return true; // Handled by agent.
			}

			return false; // Unable to create agent.
		}

		// It's an alert?
		if(strcasecmp(node.name(),"alert") == 0) {
			push_back(node,Alert::Factory::build(*this,node));
			return true; // Handled by alert.
		}

		// It's an action?
		if(strcasecmp(node.name(),"action") == 0 || strcasecmp(node.name(),"script") == 0) {
			push_back(node,Action::Factory::build(node,true));
			return true; // Handled by action.
		}

		return false;
	}

}
