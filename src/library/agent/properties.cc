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
 #include <udjat/tools/object.h>
 #include <udjat/tools/string.h>

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	bool Abstract::Agent::getProperty(const char *key, std::string &value) const {

		// Agent name
		if( !strcasecmp(key,"agent.name") ) {
			value = name();
			return true;
		}

		// Agent value
		if( !(strcasecmp(key,"value") && strcasecmp(key,"agent.value")) ) {
			value = to_string();
			return true;
		}

		// Agent path.
		if( !(strcasecmp(key,"path") && strcasecmp(key,"agent.path")) ) {
			value = path();
			return true;
		}

		// State properties
		if(!strncasecmp(key,"state.",6)) {
			if(state()->getProperty(key+6,value)) {
				return true;
			}
		}

		if(Object::getProperty(key, value))
			return true;

		// Not found, search children
		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children.agents) {
				if(!strcasecmp(key,child->name())) {
					value = child->to_string();
					return true;
				}
			}
		}

		// Not found, search related objects.
		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto object : children.objects) {
				if(object->getProperty(key,value)) {
					return true;
				}
			}
		}

		return false;

	}

 }
