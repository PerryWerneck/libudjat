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
 #include <udjat/tools/intl.h>
 #include <udjat/tools/variant.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/http/schema.h>
 #include <udjat/agent.h>
 #include <mutex>

 using namespace std;

 namespace Udjat {

	bool Abstract::Agent::schema(const char *path, Schema::Method &schema) const noexcept {
		Object::schema(path,schema);
		schema.add(
			Schema::Method::Item{HTTP::Head, Authentication::None}	// Allow 'head' requests for agents.
		);
		return true;
	}

	bool Abstract::Agent::schema(const HTTP::Method method, const char *path, Schema::Input &schema) const noexcept {
		return Object::schema(method,path,schema);
	}

	bool Abstract::Agent::schema(const HTTP::Method method, const char *path, Schema::Output &schema) const noexcept {

		auto rc = Object::schema(method,path,schema);

		if(method == HTTP::Get) {
			schema.add(
				Schema::Item{ "path",			Schema::String,		_("The agent path")	},
				Schema::Item{ "message",		Schema::String,		_("The Current state text") },
				Schema::Item{ "state",			Schema::String,		_("The current state value") },
				Schema::Item{ "statename",		Schema::String,		_("The current state name") },
				Schema::Item{ "stateicon",		Schema::Icon,		_("The current state icon") },
				Schema::Item{ "timestamp",		Schema::Timestamp,	_("Timestamp of the last state change")	}
			);
			return true;
		}

		return rc;
	}

	bool Abstract::Agent::get_property(const char *key, Value &value) const {

		// Agent name
		if( !strcasecmp(key,"agent.name") ) {
			value = name();
			return true;
		}

		// Agent value
		if( !(strcasecmp(key,"value") && strcasecmp(key,"agent.value")) ) {
			get(value);
			return true;
		}

		if( !(strcasecmp(key,"path") && strcasecmp(key,"agent.path")) ) {
			value = path();
			debug("path='",value.c_str(),"'");
			return true;
		}

		if(!strcasecmp(key,"state")) {
			return state()->get_property("level",value);
		}

		if(!strcasecmp(key,"statename")) {
			return state()->get_property("levelname",value);
		}

		if(!strcasecmp(key,"stateicon")) {
			return state()->get_property("icon",value);
		}

		if(!strcasecmp(key,"timestamp")) {
			value = TimeStamp{current_state.timestamp};
			return true;
		}

		if(!strcasecmp(key,"message")) {
			return state()->get_property("body",value);
		}

		for(const char *prop : { "level", "levelname" }) {
			if( !strcasecmp(key,prop) ) {
				return state()->get_property(prop,value);
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
