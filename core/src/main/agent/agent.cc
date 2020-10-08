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
 #include <udjat/event.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::recursive_mutex Abstract::Agent::guard;

	Abstract::Agent::Agent(Agent *p) : parent(p), state(Abstract::Agent::find_state()) {

		try {

			Config::File & config = Config::File::getInstance();

			update.timer		= config.get("agent-defaults","update-timer",update.timer);
			update.on_demand	= config.get("agent-defaults","update-on-demand",update.timer == 0);
			update.next			= time(nullptr) + config.get("agent-defaults","delay-on-startup",update.timer);

		} catch(const std::exception &e) {

			update.next	= time(nullptr) + update.timer;
			error("Can't get agent defaults: {}",e.what());

		}

	}

	Abstract::Agent::Agent(Agent *parent, const pugi::xml_node &node) : Abstract::Agent(parent) {

		this->name = Factory::validate_name(node.attribute("name").as_string());

#ifdef DEBUG
		cout << "Creating " << this->name << endl;
#endif // DEBUG

		this->update.timer = node.attribute("update-timer").as_uint(this->update.timer);

		this->update.on_demand = node.attribute("update-on-demand").as_bool(this->update.timer == 0);

		time_t delay = node.attribute("delay-on-startup").as_uint(0);
		if(delay)
			this->update.next = time(nullptr) + delay;

		this->href = Udjat::getAttribute(node,"href").as_string();

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

	void Abstract::Agent::start() {

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

		// Update state.
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

	void Abstract::Agent::get(Json::Value UDJAT_UNUSED(&value)) {
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

	Request & Abstract::Agent::get(const char UDJAT_UNUSED(*name), Request &request) {
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

	void Abstract::Agent::append_state(const pugi::xml_node UDJAT_UNUSED(&node)) {
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


}
