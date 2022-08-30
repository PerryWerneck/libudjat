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

 #include "private.h"
 #include <udjat/factory.h>

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	bool Abstract::Agent::ChildFactory(const pugi::xml_node &node) {
		return ChildFactory(node.name(),node);
	}

	bool Abstract::Agent::ChildFactory(const char *type, const pugi::xml_node &node) {

		return Factory::for_each(type,[this,&node](Factory &factory){

			try {

				lock_guard<std::recursive_mutex> lock(guard);

				auto agent = factory.AgentFactory(*this,node);
				if(agent) {
					agent->parent = this;
					agent->Object::set(node);
					agent->setup(node);
					children.agents.push_back(agent);
					return true;
				}

				auto object = factory.ObjectFactory(*this,node);
				if(object) {
					object->setup(node);
					children.objects.push_back(object);
					return true;
				}

				auto alert = factory.AlertFactory(*this,node);
				if(alert) {
					alert->setup(node);
					push_back(alert);
					return true;
				}

				return factory.push_back(node);


			} catch(const std::exception &e) {

				factory.error() << "Error '" << e.what() << "' parsing node <" << node.name() << ">" << endl;

			} catch(...) {

				factory.error() << "Unexpected error parsing node <" << node.name() << ">" << endl;

			}

			return false;

		});

	}

	void Abstract::Agent::insert(std::shared_ptr<Agent> child) {
		push_back(child);
	}

	void Abstract::Agent::push_back(std::shared_ptr<Agent> child) {

		lock_guard<std::recursive_mutex> lock(guard);

		if(child->parent) {
			throw runtime_error(string{"Agent '"} + child->name() + "' is child of '" + child->parent->name() + "'");
		}

		child->parent = this;
		children.agents.push_back(child);

	}

	/// @brief Insert object.
	void Abstract::Agent::push_back(std::shared_ptr<Abstract::Object> object) {
		lock_guard<std::recursive_mutex> lock(guard);
		children.objects.push_back(object);
	}

	/// @brief Remove object.
	void Abstract::Agent::remove(std::shared_ptr<Abstract::Object> object) {
		lock_guard<std::recursive_mutex> lock(guard);
		children.objects.remove(object);
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::find(const char *path, bool required, bool autoins) {

		lock_guard<std::recursive_mutex> lock(guard);

		if(!(path && *path)) {
			throw runtime_error("Invalid request");
		}

		if(*path == '/')
			path++;

		// Get name length.
		size_t length;
		const char *ptr = strchr(path,'/');
		if(!ptr) {
			length = strlen(path);
		} else {
			length = (ptr - path);
		}

		{
			for(auto child : children.agents) {

				if(strncasecmp(child->name(),path,length))
					continue;

				if(ptr && ptr[1]) {
					return child->find(ptr+1,required,autoins);
				}

				return child;
			}

		}

		if(autoins) {
			string name{path,length};
			auto child = make_shared<Abstract::Agent>(Quark(string(path,length)).c_str());
			push_back(child);

			if(ptr && ptr[1]) {
				return child->find(ptr+1,required,autoins);
			}

			return child;
		}

		if(required) {
			throw system_error(ENOENT,system_category(),string{"Can't find agent '"} + path);
		}

		return shared_ptr<Abstract::Agent>();

	}

	void Abstract::Agent::for_each(std::function<void(Abstract::Agent &agent)> method) {

		lock_guard<std::recursive_mutex> lock(guard);

		for(auto child : children.agents) {
			child->for_each(method);
		}

		method(*this);

	}

	void Abstract::Agent::for_each(std::function<void(std::shared_ptr<Abstract::Agent> agent)> method) {

		lock_guard<std::recursive_mutex> lock(guard);

		for(auto child : children.agents) {

			if(!child->children.agents.empty()) {
				child->for_each(method);
			}

			method(child);

		}

	}

	void Abstract::Agent::for_each(std::function<void(std::shared_ptr<EventListener> listener)> method) {

		lock_guard<std::recursive_mutex> lock(guard);

		for(auto listener : listeners) {

			method(listener);

		}

	}


 }
