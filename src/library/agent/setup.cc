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
 #include <udjat/agent/abstract.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/interface.h>
 #include <udjat/alert.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::setup(const XML::Node &root) {

		Controller::setup_properties(*this,root);

		// Load children.
		for(const XML::Node &node : root) {

			// It's an attribute?
			if(is_reserved(node) || !is_allowed(node)) {
				continue;
			}

			// It's a state?
			if(strcasecmp(node.name(),"state") == 0) {

				try {

					auto state = StateFactory(node);
					if(state) {
						for(XML::Node child : node) {

							if(is_reserved(node) || !is_allowed(node)) {
								continue;
							}
							state->push_back(child);
						}
					} else {
						Logger::String{"Unable to create agent state"}.error(name());
					}

				} catch(const std::exception &e) {

					Logger::String{"Cant parse <",node.name(),">: ",e.what()}.error(name());

				} catch(...) {

					Logger::String{"Unexpected error parsing <",node.name(),">"}.error(name());

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

				auto agent = Abstract::Agent::Factory::build(*this,node);
				if(agent) {
					agent->Object::set(node);
					agent->setup(node);
					push_back(agent);
					continue;
				}

			}

			// It's an alert?
			if(strcasecmp(node.name(),"alert") == 0) {
				try {
					push_back(node,Alert::Factory::build(*this,node));
				} catch(const std::exception &e) {
					Logger::String{"Alert creation failure: ",e.what()}.error(node.attribute("name").as_string(name()));
				}
				continue;
			}

			// It's an action?
			if(strcasecmp(node.name(),"action") == 0 || strcasecmp(node.name(),"script") == 0) {
				try {
					push_back(node,Action::Factory::build(node,true));
				} catch(const std::exception &e) {
					Logger::String{"Action creation failure: ",e.what()}.error(name());
				}
				continue;
			}

			// It's an interface?
			if(strcasecmp(node.name(),"interface") == 0) {
				Interface::Factory::build(node);
				continue;
			}

			if(strcasecmp(node.name(),"init") == 0) {
				try {
					auto action = Action::Factory::build(node,true);
					if(action) {
						action->call(node);
					}
				} catch(const std::exception &e) {
					Logger::String{"Initialization failure: ",e.what()}.error(node.attribute("name").as_string("action"));
				} catch(...) {
					Logger::String{"Initialization failure: Unexpected error"}.error(node.attribute("name").as_string("action"));
				}
				continue;
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
