/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <private/agent.h>
 #include <udjat/tools/object.h>
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/report.h>

 namespace Udjat {

	template <typename T>
	bool getFromPath(const Abstract::Agent &agent, const char *path, T &value) {

		if(!(path && *path)) {
			debug("Got agent '",agent.name(),"', asking for properties");
			return agent.get(value);
		}

		if(*path == '/') {
			path++;
		}

		size_t len;
		const char *next = strchr(path,'/');
		if(next) {
			len = path-next;
			next++;
		} else {
			len = strlen(path);
			next = "";
		}

		debug("Searching for child '",string{path,len}.c_str(),"' on agent '",agent.name(),"'");

		for(auto child : agent) {
			if(!strncasecmp(path,child->name(),len)) {
				debug("Found child '",child->name(),"' (next='",next,"')");
				return child->getProperties(next,value);
			}
		}

		/*
		const char *name = path;
		size_t len;

		path = strchr(path,'/');
		if(path) {
			len = path-name;
			path++;
		} else {
			len = strlen(name);
			path = "";
		}

		debug("Searching for child '",string{name,len}.c_str(),"' on agent '",agent.name(),"'");

		for(auto child : agent) {
			if(!strncasecmp(name,child->name(),len)) {
				debug("Found child '",child->name(),"'");
				return child->getProperties(path,value);
			}
		}

		debug("Cant find child '",string{name,len}.c_str(),"'");
		*/

		return false;
	}

	bool Abstract::Agent::getProperties(const char *path, Value &value) const {

		if(getFromPath(*this,path,value)) {
			return true;
		}

		if(!strcasecmp(path,"state") && strlen(path) == 5 && current_state.selected) {
			current_state.selected->getProperties(value);
			return true;
		}

		if(Abstract::Object::getProperty(path,value)) {
			debug("Found property '",path,"'");
			return true;
		}

		debug("Unable to find property '",path,"'");
		return false;

	}

	bool Abstract::Agent::getProperties(const char *path, Udjat::Response::Value &value) const {
		if(getFromPath(*this,path,value)) {
			return true;
		}
		debug("Cant find Response::Value('",path,"'), trying with Udjat::Value('",path,"')");
		return getProperties(path,(Udjat::Value &) value);
	}

	bool Abstract::Agent::getProperties(const char *path, Udjat::Response::Table &report) const {

		if(getFromPath(*this,path,report)) {
			return true;
		}

		if(!strcasecmp(path,"states")) {
			getStates(report);
			return true;
		}

		return false;
	}


 }


