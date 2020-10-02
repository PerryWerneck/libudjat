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
 #include <udjat/factory.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::recursive_mutex Abstract::Agent::guard;

	static std::shared_ptr<Abstract::Agent> root_agent;

	void set_root_agent(std::shared_ptr<Abstract::Agent> agent) {
		root_agent = agent;
	}

	Abstract::Agent::Agent(Agent *p) : parent(p), state(Abstract::Agent::find_state()) {

		try {

			Config::File & config = Config::File::getInstance();

			update.on_demand	= config.get("agent_defaults","update-on-demand",update.on_demand);
			update.timer		= config.get("agent_defaults","update-timer",update.timer);
			update.next			= time(nullptr) + config.get("agent_defaults","delay-on-startup",update.timer);

		} catch(const exception &e) {

			cerr << e.what() << endl;

		}

	}

	Abstract::Agent::Agent(Agent *parent, const pugi::xml_node &node) : Abstract::Agent(parent) {

		this->name = Factory::validate_name(node.attribute("name").as_string());

#ifdef DEBUG
		cout << "Creating " << this->name << endl;
#endif // DEBUG

		this->update.timer = node.attribute("update-timer").as_uint(this->update.timer);
		this->update.next = time(nullptr) + node.attribute("delay-on-startup").as_uint(this->update.timer);
		this->update.on_demand = node.attribute("update-on-demand").as_bool(this->update.timer == 0);

		this->href = Udjat::getAttribute(node,"href").as_string();

	}

	Abstract::Agent::~Agent() {

		// Deleted! My children are now orphans.
		for(auto child : children) {
			child->parent = nullptr;
		}

	}

	void Abstract::Agent::start() {

		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {
			child->start();
		}

		// Update state.
		auto new_state = find_state();

		// Check for children state
		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children) {

				auto child_state = child->getState();

				/*
#ifdef DEBUG
				cout	<< "Child state is \""
						<< child_state->getSummary() << "\" (" << child_state->getLevel() << ")"
						<< " My level is \"" << this->state->getSummary() << "\" (" << this->state->getLevel() << ")" << endl;
#endif // DEBUG
				*/

				if(child_state && child_state->getLevel() > new_state->getLevel()) {
					new_state = child_state;
				}

			}

		}

		activate(new_state);

	}

	void Abstract::Agent::stop() {

		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {
			child->stop();
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

				if(ptr) {
					return child->find(ptr+1);
				}

				return child;
			}

		}

		throw system_error(ENOENT,system_category(),"Agent search has failed");

	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::find(const std::vector<std::string> &path) {

		std::shared_ptr<Abstract::Agent> agent;

		for(auto node : path) {

			if(!agent) {
				agent = this->find(node.c_str());
			} else {
				agent = agent->find(node.c_str());
			}

		}

		return agent;

	}

	void Abstract::Agent::foreach(std::function<void(Abstract::Agent &agent)> method) {

		lock_guard<std::recursive_mutex> lock(guard);

		for(auto child : children) {
			child->foreach(method);
		}

		if(parent)
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

	void Abstract::Agent::get(Json::Value &value) {
		chk4refresh();
	}

	Request & Abstract::Agent::setup(Request &request) {

		chk4refresh(true);

		if(update.expires && update.expires > time(nullptr))
			request.setExpirationTimestamp(update.expires);

		if(update.last)
			request.setModificationTimestamp(update.last);

		return request;
	}

	Request & Abstract::Agent::get(const char *name, Request &request) {
		return setup(request);
	}

	Request & Abstract::Agent::get(Request &request) {

		setup(request);

		for(auto child : children) {
			child->get(child->getName(),request);
		}

		return request;
	}

	Json::Value Abstract::Agent::as_json() {

		Json::Value node;

		chk4refresh(true);

		lock_guard<std::recursive_mutex> lock(guard);

		if(children.empty()) {
			get(node);
		} else {

			for(auto child : children) {
				node[child->getName()] = child->as_json();
			}

		}

		return node;

	}

	void Abstract::Agent::append_state(const pugi::xml_node &node) {
		string str("Can't append state on agent \"");
		str += this->name.c_str();
		str += "\": It's a non-typed agent";
		throw runtime_error(str);
	}

	std::shared_ptr<Abstract::State> Abstract::Agent::find_state() const {

		// Default method should return a common "undefined" state.

		static std::shared_ptr<Abstract::State> state;

		if(!state) {
			state = std::make_shared<Abstract::State>(Abstract::State::undefined,"");
		}

		return state;

	}

	std::shared_ptr<Abstract::Agent> find_agent(const char *path) {

		if(!root_agent)
			throw runtime_error("Agent controller is non existant or inactive");

		if(path && *path)
			return root_agent->find(path);

		return root_agent;

	}


}
