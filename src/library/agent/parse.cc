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
 #include <udjat/agent/state.h>
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

	bool Abstract::Agent::parse(const XML::Node &node) {

		if(Udjat::Object::parse(node)) {
			return true;
		}

		// It's a state?
		if(strcasecmp(node.name(),"state") == 0) {

			auto state = StateFactory(node);
			if(state) {
				for(const XML::Node &child : node) {
					state->parse(child);
				}
			} else {
				Logger::String{"Unable to create agent state"}.error(name());
			}
			
			return true;
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

#ifdef DEBUG 
		Logger::String{"Unexpected node <Agent::",node.name(),">"}.warning(name());
#endif // DEBUG

		return false;
	}

}
