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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/agent/abstract.h>
 #include <private/agent.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

	bool Abstract::Agent::push_back(std::shared_ptr<Abstract::Object> object) {

		// Is this an agent?
		{
			auto agent = std::dynamic_pointer_cast<Abstract::Agent>(object);
			if(agent) {

				debug("Pushing agent ",object->name()," into agent ",name());
				lock_guard<std::recursive_mutex> lock(guard);

				if(agent->parent) {
					throw runtime_error(Logger::Message{"Agent {} is child of {}",agent->name(),agent->parent->name()});
				}

				agent->parent = this;

				if(!agent->current_state.selected) {
					agent->current_state.set(agent->computeState());
				}

				children.agents.push_back(agent);

				return true;
			}
		}

		// Is this an activatable object? 
		{
			auto activatable = std::dynamic_pointer_cast<Activatable>(object);
			if(activatable) {
				debug("Pushing activatable ",object->name()," into agent ",name());
				lock_guard<std::recursive_mutex> lock(guard);


				return true;
			}
		}
		
		// Generic object.
		{
			debug("Pushing generic object ",object->name()," into agent ",name());
			lock_guard<std::recursive_mutex> lock(guard);
			children.objects.push_back(object);
			return true;
		}

	}


	bool Abstract::Agent::push_back(const XML::Node &node, std::shared_ptr<Abstract::Object> object) {

		// Is this an activatable object?
		{
			auto activatable = std::dynamic_pointer_cast<Activatable>(object);
			if(activatable) {
				debug("Pushing activatable ",object->name()," into agent ",name()," from path ",node.path());
				lock_guard<std::recursive_mutex> lock(guard);
				listeners.emplace_back(EventFactory(node,"trigger-event"),activatable);
				return true;
			}
		}

		return push_back(object);
	
	}

	void Abstract::Agent::push_back(const Abstract::Agent::Event event, std::shared_ptr<Activatable> activatable) {
		lock_guard<std::recursive_mutex> lock(guard);
		listeners.emplace_back(event,activatable);
	}

	bool Abstract::Agent::push_back(const XML::Node &node, std::shared_ptr<Activatable> alert) {
		return push_back(node, std::static_pointer_cast<Abstract::Object>(alert));
	}


 }
