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

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/factory.h>
 #include <udjat/alert/abstract.h>

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	void Abstract::Agent::insert(std::shared_ptr<Agent> child) {
		push_back(child);
	}

	void Abstract::Agent::push_back(std::shared_ptr<Abstract::Agent> child) {

		lock_guard<std::recursive_mutex> lock(guard);

		if(child->parent) {
			throw runtime_error(string{"Agent '"} + child->name() + "' is child of '" + child->parent->name() + "'");
		}

		child->parent = this;

		if(!child->current_state.selected) {
			child->current_state.set(child->computeState());
		}

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

	bool Abstract::Agent::empty() const noexcept {
		return children.agents.empty();
	}

	const Abstract::Agent * Abstract::Agent::find(const char **path) const {

		const char *ptr = *path;

		if(!(ptr && *ptr)) {
			throw system_error(EINVAL,system_category());
		}

		if(*ptr == '/') {
			ptr++;
		}

		*path = ptr;

		debug("PATH='",ptr,"'");

		size_t length;
		ptr = strchr(*path,'/');
		if(!ptr) {
			length = strlen(*path);
		} else {
			length = ptr - (*path);
		}

		debug("PATH='",*path,"' length=",length);

		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children.agents) {
				if(!strncasecmp(child->name(),*path,length)) {
					*path = (*path) + length;
					debug("Found '",child->name(),"'");
					return child.get();
				}
			}
		}

		return nullptr;

	}

	bool Abstract::Agent::getProperties(const char *path, Value &value) const {

		debug("ME='",name(),"' path='",path,"'");

		if(!*path) {
			getProperties(value);
			return true;
		}

		auto agent = find(&path);
		if(agent) {
			return agent->getProperties(path,value);
		}

		return false;
	}

	std::shared_ptr<Abstract::State> Abstract::Agent::state(const char *path) const {

		/*
		lock_guard<std::recursive_mutex> lock(guard);

		size_t length = getlength(path);

		for(auto child : children.agents) {

			if(strncasecmp(child->name(),path,length))
				continue;

			if(*(path+length) == '/' && *(path+length+1)) {
				return child->state(path+length+1);
			}

			return child->state();

		}
		*/

		throw system_error(ENOENT,system_category());

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

 }
