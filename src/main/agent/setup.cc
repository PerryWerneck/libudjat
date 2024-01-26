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

	void Abstract::Agent::setup(const XML::Node &root, bool upsearch) {

		Controller::setup_properties(*this,root);

		// Load children.
		for(const XML::Node &node : root) {

			// It's an attribute?
			if(!(strcasecmp(node.name(),"attribute") && strcasecmp(node.name(),"attribute-list"))) {
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
					agent->setup(node,true);
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

			// Run factories.
			bool success = Udjat::Factory::for_each([this,&node](Udjat::Factory &factory) {

				if(!factory.probe(node)) {
					return false;
				}

				try {

					// It's an agent?
					{
						auto agent = factory.AgentFactory(*this,node);
						if(agent) {
							agent->Object::set(node);
							agent->setup(node,true);
							push_back(agent);
							return true;
						}
					}

					// It's an alert?
					{
						auto alert = factory.AlertFactory(*this, node);
						if(alert) {
							alert->setup(node);
							push_back(node,alert);
							return true;
						}

					}

					// It's an object?
					{
						auto object = factory.ObjectFactory(*this,node);
						if(object) {
							lock_guard<std::recursive_mutex> lock(guard);
							object->setup(node);
							children.objects.push_back(object);
							return true;
						}

					}

					// Try generic parser.
					return factory.generic(*this, node);

				} catch(const std::exception &e) {

					factory.error() << "Error '" << e.what() << "' parsing node <" << node.name() << ">" << endl;

				} catch(...) {

					factory.error() << "Unexpected error parsing node <" << node.name() << ">" << endl;

				}

				return false;

			});

			if(!success && Logger::enabled(Logger::Debug)) {
				Logger::String{
					"Ignoring node <",node.name(),">"
				}.write(Logger::Debug,PACKAGE_NAME);
			}

		}

		// Load default values.
		//
		// Search for common states & alerts.
		//
		// Example:
		//
		// <service-defaults>
		//		<alert type='bla' ... />
		// </service-defaults>
		//
		// <service name='bla1' ... />
		// <service name='bla2' ... />
		//
		// The alert type 'bla' will be loaded in both services.
		//
		if(upsearch) {

			string nodename{root.attribute("type").as_string(root.name())};
			nodename += "-defaults";

			for(XML::Node node = root.parent(); node; node = node.parent()) {

				if(!is_allowed(node)) {
					continue;
				}

				for(auto child = node.child(nodename.c_str()); child; child = child.next_sibling(nodename.c_str())) {
					if(is_allowed(child)) {
						setup(child,false);
					}
				}
			}
		}
	}

}
