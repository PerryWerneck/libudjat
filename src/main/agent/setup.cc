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

	void Abstract::Agent::setup(const pugi::xml_node &root) {

		Controller::setup_properties(*this,root);
		Controller::setup_states(*this,root);
		Controller::setup_alerts(*this,root);
		Controller::setup_children(*this,root);

		// Search for common states & alerts.
		string nodename{root.name()};
		nodename += "-defaults";
		for(XML::Node node = root.parent(); node; node = node.parent()) {
			for(auto child = node.child(nodename.c_str()); child; child = child.next_sibling(nodename.c_str())) {
				Controller::setup_states(*this,child);
				Controller::setup_alerts(*this,child);
			}
		}

	}

}
