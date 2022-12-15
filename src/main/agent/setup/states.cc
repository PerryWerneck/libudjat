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
 * @brief Implements the agent state loader.
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

	void Abstract::Agent::Controller::setup_states(Abstract::Agent &agent, const pugi::xml_node &root) noexcept {

		// Load agent states.
		Abstract::Object::for_each(root,"state","states",[&agent](const pugi::xml_node &node){
			try {

				auto state = agent.StateFactory(node);
				if(state) {
					state->setup(node);
				} else {
					agent.error() << "Unable to create child state" << endl;
				}

			} catch(const std::exception &e) {

				agent.error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

			} catch(...) {

				agent.error() << "Unexpected error parsing <" << node.name() << ">" << endl;

			}
		});

	}

}
