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
 * @brief Implement agent alerts setup.
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

	std::shared_ptr<Abstract::Alert> Abstract::Agent::AlertFactory(const pugi::xml_node &node) {
		return Udjat::AlertFactory(*this,node);
	}

	/*
	void Abstract::Agent::Controller::setup_alerts(Abstract::Agent &agent, const pugi::xml_node &root) noexcept {

		// Load agent alerts.
		Abstract::Object::for_each(root,"alert","alerts",[&agent](const pugi::xml_node &node){

			// Create alerts.
			try {

				auto alert = agent.AlertFactory(node);
				if(alert) {

					// Got alert, setup and insert it on agent.

					alert->setup(node);

					switch(String{node,"trigger-event","default"}.select("state-change","level-change","value-change","ready-state","not-ready-state",NULL)) {
					case 0:	// state-change
						agent.push_back(Event::STATE_CHANGED,alert);
						break;

					case 1:	// level-change
						agent.push_back(Event::LEVEL_CHANGED,alert);
						break;

					case 2:	// value-change
						agent.push_back(Event::VALUE_CHANGED,alert);
						break;

					case 3:	// ready-state
						agent.push_back(Event::READY,alert);
						break;

					case 4:	// not-ready-state
						agent.push_back(Event::NOT_READY,alert);
						break;

					default:
						debug("Using alert activation from agent '",agent.name(),"'");
						agent.push_back(node, alert);
					}

				} else {

					agent.error() << "Unable to create alert" << endl;

				}

			} catch(const std::exception &e) {

				agent.error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

			} catch(...) {

				agent.error() << "Unexpected error parsing <" << node.name() << ">" << endl;

			}

		});
	}
	*/
}
