/**
 * @file src/core/agent/agent.cc
 *
 * @brief Implements the abstract agent methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/event.h>
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
			update.notify 		= config.get("agent-defaults","notify-on-value-change",update.notify);

		} catch(const std::exception &e) {

			update.next	= time(nullptr) + update.timer;
			cerr << "Agent\tError '" << e.what() << "' loading defaults" << endl;

		}

	}

	Abstract::Agent::Agent(const pugi::xml_node &node) : Abstract::Agent() {

		this->load(node);

#ifdef DEBUG
		cout << "Agent '" << this->name << "' created" << endl;
#endif // DEBUG

	}

	Abstract::Agent::~Agent() {

		// Deleted! My children are now orphans.
		for(auto child : children) {
			child->parent = nullptr;
		}

		// Delete my events.
		for(auto event : events) {
			delete event;
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
		cout << "Starting agent '" << name << "'" << endl;
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

	std::shared_ptr<Abstract::Agent> Abstract::Agent::find(const char *path) {

		if(!(path && *path)) {
			throw runtime_error("Invalid request");
		}

		if(*path == '/')
			path++;

		size_t length;
		const char *ptr = strchr(path,'/');
		if(!ptr) {
			length = strlen(path);
		} else {
			length = (ptr - path);
		}

		{
			lock_guard<std::recursive_mutex> lock(guard);

			for(auto child : children) {
				if(strncasecmp(child->name.c_str(),path,length))
					continue;

				if(ptr && ptr[1]) {
					return child->find(ptr+1);
				}

				return child;
			}

		}

		throw system_error(ENOENT,system_category(),"Agent search has failed");

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
				values["teste"] = child->getName();
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

	std::shared_ptr<Abstract::State> Abstract::Agent::append_state(const pugi::xml_node &node) {
		string str("Can't append state on agent '");
		str += this->name.c_str();
		str += "'";
		throw runtime_error(str);
	}

	std::shared_ptr<Abstract::State> Abstract::Agent::find_state() const {

		// Default method should return the current state with no change.
		return this->state;

	}


}
