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
 #include <udjat/tools/schema.h>
 #include <udjat/agent.h>
 #include <mutex>

 using namespace std;

 namespace Udjat {

	bool Abstract::Agent::output_schema(const char *path, Schema &schema) const noexcept {

		Object::output_schema(path,schema);

		schema.append(
			Schema::Item{ "path",			Udjat::Value::String		},
			Schema::Item{ "state_icon",		Udjat::Value::Icon			},
			Schema::Item{ "state_body",		Udjat::Value::String		},
			Schema::Item{ "state_level",	Udjat::Value::String		}
		);

		return true;
	}

	bool Abstract::Agent::get_property(const char *key, std::string &value) const {

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
		if(!strncasecmp(key,"state_",6)) {
			if(state()->get_property(key+6,value)) {
				return true;
			}
		}

		if(Object::get_property(key, value))
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
				if(object->get_property(key,value)) {
					return true;
				}
			}
		}

		return false;

	}

 }
