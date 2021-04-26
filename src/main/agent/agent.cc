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
 *
 * @brief Implements the abstract agent methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <cstring>
 #include <udjat.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::recursive_mutex Abstract::Agent::guard;

	Abstract::Agent::Agent(const char *name, const char *label, const char *summary) : state(get_default_state()) {

		if(name && *name) {
			this->name = name;
			this->label = (label ? label : name);
		}

		if(summary && *summary) {
			this->summary = summary;
		}

		try {

			Config::File & config = Config::File::getInstance();

			update.timer		= config.get("agent-defaults","update-timer",update.timer);
			update.on_demand	= config.get("agent-defaults","update-on-demand",update.timer == 0);
			update.next			= time(nullptr) + config.get("agent-defaults","delay-on-startup",update.timer);
			update.failed 		= config.get("agent-defaults","delay-when-failed",update.failed);

		} catch(const std::exception &e) {

			update.next	= time(nullptr) + update.timer;
			cerr << "Agent\tError '" << e.what() << "' loading defaults" << endl;

		}

	}

	Abstract::Agent::~Agent() {

		// Deleted! My children are now orphans.
		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {
			child->parent = nullptr;
		}

	}

	void Abstract::Agent::insert(std::shared_ptr<Agent> child) {
		lock_guard<std::recursive_mutex> lock(guard);

		if(child->parent) {
			throw runtime_error("Agent already has a parent");
		}

#ifdef DEBUG
		cout << "Inserting agent '" << child->getName() << "' in parent '" << this->getName() << endl;
#endif // DEBUG

		child->parent = this;
		children.push_back(child);

	}

	void Abstract::Agent::start() {

#ifdef DEBUG
		cout << name << "\tStarting agent" << endl;
#endif // DEBUG

		// Start children
		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children) {

				try {

					child->start();

				} catch(const std::exception &e) {

					child->failed(e,"Agent startup has failed");

				}
			}
		}

		// Update agent state.
		{
			auto new_state = find_state();

			// Check for children state
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children) {
					if(child->state && child->state->getLevel() > new_state->getLevel()) {
						new_state = child->state;
					}
				}
			}

			activate(new_state);
		}

	}

	void Abstract::Agent::stop() {

		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {

			try {

				child->stop();

			} catch(const exception &e) {

				child->failed(e,"Agent stop has failed");

			}
		}
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

				if(strncasecmp(child->name.c_str(),path,length))
					continue;

				if(ptr && ptr[1]) {
					return child->find(ptr+1,required,autoins);
				}

				return child;
			}

		}

		if(autoins) {
			string name{path,length};
			auto child = make_shared<Abstract::Agent>(string(path,length).c_str());
			insert(child);

			if(ptr && ptr[1]) {
				return child->find(ptr+1,required,autoins);
			}

			return child;
		}

		if(required) {
			throw system_error(ENOENT,system_category(),string{"Can't find agent '"} + path);
		}

		return make_shared<Abstract::Agent>();

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

	Json::Value & Abstract::Agent::setup(const Request &request, Response &response) {

		chk4refresh(true);

		if(update.expires && update.expires > time(nullptr))
			response.setExpirationTimestamp(update.expires);

		if(update.last)
			response.setModificationTimestamp(update.last);

		return response;
	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Abstract::Agent::get(const char *name, Json::Value &value) {
		// It's just a placeholder here. Don't set any value.
	}
	#pragma GCC diagnostic pop

	std::string Abstract::Agent::to_string() const {
		throw system_error(ENOTSUP,system_category(),string{"Can't get value for agent'"} + getName() + "'");;
	}

	void Abstract::Agent::get(Json::Value &value, const bool children, const bool state) {

		get("value",value);

		value["name"] = this->getName();
		value["summary"] = this->summary.c_str();
		value["label"] = this->label.c_str();
		value["uri"] = this->uri.c_str();
		value["icon"] = this->icon.c_str();

		// Get state
		if(state) {
			auto state = Json::Value(Json::objectValue);
			this->state->get(state);
			value["state"] = state;
		}

		// Get children values
		if(children) {
			auto cvalues = Json::Value(Json::objectValue);
			for(auto child : this->children) {
				auto values = Json::Value(Json::objectValue);
				child->get(values,false,true);
				cvalues[child->getName()] = values;
			}
			value["children"] = cvalues;
		}

	}

	void Abstract::Agent::get(const Request &request, Response &response) {

#ifdef DEBUG
		cout << "Getting agent '" << getName() << "'" << endl;
#endif // DEBUG

		setup(request,response);

		// Get agent value
		get( (Json::Value &) response, false, true);

	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Abstract::Agent::append_state(const pugi::xml_node &node) {
		string str{"Can't append state on agent '"};
		str += this->name.c_str();
		str += "'";
		throw runtime_error(str);
	}
	#pragma GCC diagnostic pop

	std::shared_ptr<Abstract::State> Abstract::Agent::find_state() const {

		// Default method should return the current state with no change.
		return this->state;

	}

	void Abstract::Agent::expand(std::string &text) const {

		Udjat::expand(text,[this](const char *key) {

			// Agent value
			if( !(strcasecmp(key,"value") && strcasecmp(key,"agent.value")) ) {
				return to_string();
			}

			// Agent properties.
			{
				struct {
					const char *key;
					const Quark &value;
				} values[] = {
					{ "agent.name",		this->name		},
					{ "agent.label",	this->label		},
					{ "agent.summary",	this->summary	},
					{ "agent.uri", 		this->uri		},
					{ "agent.icon", 	this->icon		}
				};

				for(size_t ix = 0; ix < (sizeof(values)/sizeof(values[0]));ix++) {

					if(!strcasecmp(values[ix].key,key)) {
						return string(values[ix].value.c_str());
					}

				}
			}

			if(this->state) {

				struct {
					const char *key;
					const Quark &agent;
					const Quark &state;
				} values[] = {
					{
						"summary",
						this->summary,
						this->state->getSummary()
					},
					{
						"uri",
						this->uri,
						this->state->getUri()
					}
				};

				for(size_t ix = 0; ix < (sizeof(values)/sizeof(values[0]));ix++) {

					if(!strcasecmp(values[ix].key,key)) {

						if(values[ix].state) {
							return string(values[ix].state.c_str());
						}

						return string(values[ix].agent.c_str());

					}

				}

			}

#ifdef DEBUG
			cout << "Can't find key '" << key << "'" << endl;
#endif // DEBUG
			return string{"${}"};

		});

	}


}
