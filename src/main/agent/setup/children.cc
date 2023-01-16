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
 * @file
 *
 * @brief Implements the agent setup.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/module.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/alert/activation.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

			/*
	void Abstract::Agent::Controller::setup_children(Abstract::Agent &parent, const pugi::xml_node &root) noexcept {
		for(const pugi::xml_node &node : root) {

			// Check for validation.
			if(!is_allowed(node)) {
				continue;
			}

			// Create child.
			if(strcasecmp(node.name(),"module") == 0) {
				//
				// Load module.
				//
				Module::load(node);

			} else if(strcasecmp(node.name(),"agent") == 0) {

				auto agent = Abstract::Agent::Factory(node,parent);

				if(agent) {

					agent->Object::set(node);
					agent->setup(node);

					parent.push_back(agent);

				} else {

					Logger::String{
						"Dont know how to create a child of type '",node.attribute("type").as_string(),"', ignoring"
					}.write(Logger::Debug,agent.name());

				}

			} else if(strcasecmp(node.name(),"attribute")) {

				// It's not an attribute, try generic factories.

			}

		}
	}
			*/

}
