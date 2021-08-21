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

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	void Abstract::Agent::insert(std::shared_ptr<Agent> child) {

		lock_guard<std::recursive_mutex> lock(guard);

		if(child->parent) {
			throw runtime_error("Agent already has a parent");
		}

		child->parent = this;
		children.push_back(child);

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
			for(auto child : children) {

				if(strncasecmp(child->getName(),path,length))
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
			insert(child);

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

	void Abstract::Agent::foreach(std::function<void(Abstract::Agent &agent)> method) {

		lock_guard<std::recursive_mutex> lock(guard);

		for(auto child : children) {
			child->foreach(method);
		}

		method(*this);

	}

	void Abstract::Agent::foreach(std::function<void(std::shared_ptr<Abstract::Agent> agent)> method) {

		lock_guard<std::recursive_mutex> lock(guard);

		for(auto child : children) {

			if(!child->children.empty()) {
				child->foreach(method);
			}

			method(child);

		}

	}

 }
