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
 #include <udjat/agent.h>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/object.h>
 #include <udjat/agent/state.h>
 #include <udjat/alert.h>
 #include <udjat/action.h>

 //  #include <udjat/tools/object.h>
//  #include <udjat/tools/configuration.h>
//  #include <udjat/action.h>
//  #include <udjat/tools/event.h>
//  #include <udjat/tools/mainloop.h>
//  #include <udjat/tools/logger.h>

namespace Udjat {

	bool Abstract::Agent::append_child(const Properties &props) {

		if(Udjat::Object::append_child(props)) {
			return true;
		}

		// It's a state?
		if(props == "state") {

			auto state = StateFactory(props);
			if(state) {
				state->append_children(props);
				return true; // Handled by state.
			}
			
		}

		// It's an alert? Push it as an activatable.
		if(props == "alert") {
			push_back(props,Alert::Factory::build(*this,props));
			return true; // Handled by alert.
		}

		// It's an action? Push it as an activatable.
		if(props == "action" || props == "script") {
			push_back(props,Action::Factory::build(props));
			return true; // Handled by action.
		}

		Logger::String{"Ignoring build of '",props.node_name(),"'"}.warning(name());

		return false;
	}

}
