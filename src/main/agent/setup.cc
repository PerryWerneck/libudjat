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
 #include <udjat/module/abstract.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/alert/activation.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::setup(const XML::Node &root) {

		Controller::setup_properties(*this,root);

		// Load children.
		for(const XML::Node &node : root) {

			// It's an attribute?
			if(!(strncasecmp(node.name(),"attribute",9))) {
				continue;
			}

			// Is this node allowed?
			if(!is_allowed(node)) {
				continue;
			}

			// It's a state?
			if(strcasecmp(node.name(),"state") == 0) {

				try {

					auto state = StateFactory(node);
					if(state) {
						state->setup(node);
					} else {
						error() << "Unable to create agent state" << endl;
					}

				} catch(const std::exception &e) {

					error() << "Cant parse <" << node.name() << ">: " << e.what() << endl;

				} catch(...) {

					error() << "Unexpected error parsing <" << node.name() << ">" << endl;

				}

				continue;
			}

			// It's a module?
			if(strcasecmp(node.name(),"module") == 0) {
				Module::load(node);
				continue;
			}

			// It's an agent?
			if(strcasecmp(node.name(),"agent") == 0) {

				auto agent = Abstract::Agent::Factory(*this,node);
				if(agent) {

					agent->Object::set(node);
					agent->setup(node);
					push_back(agent);
					continue;

				}

			}

			// It's an alert or action?
			if(strcasecmp(node.name(),"alert") == 0 || strcasecmp(node.name(),"action") == 0) {

				std::shared_ptr<Activatable> alert = Abstract::Alert::Factory(*this, node);

				if(alert) {
					alert->setup(node);
					if(!push_back(node,alert)) {
						push_back(alert);
					}
					continue;
				}

			}

			// Run node based factories.
			if(Udjat::Factory::for_each(node,[this,&node](Udjat::Factory &factory) {

				if(factory.NodeFactory(*this,node)) {
					return true;
				}

				return factory.NodeFactory(node);

			})) {

				continue;

			}


			// Run factories.
			if(Udjat::Factory::for_each([this,&node](Udjat::Factory &factory) {

				if(factory == node.attribute("type").as_string("default") && factory.CustomFactory(*this,node)) {
					return true;
				}

				return false;

			})) {

				continue;

			}

			if(Logger::enabled(Logger::Debug)) {
				Logger::String{"Ignoring node <",node.name(),">"}.write(Logger::Debug,PACKAGE_NAME);
			}

		}

	}

}
