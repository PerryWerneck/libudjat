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
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/report.h>

 namespace Udjat {

	const Abstract::Agent * Abstract::Agent::Controller::find(const Abstract::Agent *agent, const char **path) {

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
			lock_guard<std::recursive_mutex> lock(agent->guard);
			for(auto child : agent->children.agents) {
				if(!strncasecmp(child->name(),*path,length)) {
					ptr = (*path) + length;
					if(*ptr == '/') {
						ptr++;
					}
					*path = ptr;
					debug("Found '",child->name(),"'");
					return child.get();
				}
			}
		}

		return nullptr;

	}

	bool Abstract::Agent::getProperties(const char *path, Value &value) const {

		debug("ME='",name(),"' path='",path,"'");

		if(!(path && *path)) {
			getProperties(value);
			return true;
		}

		auto agent = Controller::find(this, &path);
		if(agent) {
			return agent->getProperties(path,value);
		}

		return false;

	}

	bool Abstract::Agent::getProperties(const char *path, std::shared_ptr<Abstract::State> &state) const {

		debug("ME='",name(),"' path='",path,"'");

		if(!*path) {
			state = this->state();
			return true;
		}

		auto agent = Controller::find(this, &path);
		if(agent) {
			return agent->getProperties(path,state);
		}

		return false;

	}

	bool Abstract::Agent::getProperties(const char *path, Udjat::Response::Table &report) const {

		if(!*path) {
			// If this method wasnt overrided theres no report, just return false here.
			return false;
		}

		auto agent = Controller::find(this, &path);
		if(agent) {
			return agent->getProperties(path,report);
		}

		return false;
	}

	bool Abstract::Agent::Controller::head(Abstract::Agent *agent, const char *path, Udjat::Abstract::Response &response) {

		debug("-[HEAD]----------------------> ME='",agent->name(),"' path='",path,"'");

		if(!*path) {

			// Found agent, get cache info.
			agent->chk4refresh(true);

			time_t now = time(nullptr);

			// Gets the minor time for the next update.
			time_t next{now+Config::Value<time_t>("agent","min-update-time",600)};

			// Gets the major time from the last update.
			time_t updated = 0;

			agent->for_each([&next,&updated](Agent &agent){

				if(agent.update.next) {
					next = std::min(next,agent.update.next);
				}

				if(agent.update.last) {
					if(updated) {
						updated = std::max(updated,agent.update.last);
					} else {
						updated = agent.update.last;
					}
				}

			});

			if(next > now) {
				response.setExpirationTimestamp(next);
			} else {
				response.setExpirationTimestamp(0);
			}

			if(updated >= now) {
				response.setModificationTimestamp(updated);
			}

			return true;
		}

		auto next = Controller::find(agent, &path);
		if(next) {
			return head(const_cast<Abstract::Agent *>(next), path, response);
		}

		auto dummy = Udjat::Value::Factory();
		return agent->getProperties(path,*dummy);

	}

 }


