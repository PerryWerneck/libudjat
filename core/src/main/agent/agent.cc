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
			update.notify 		= config.get("agent-defaults","notify-on-value-change",update.notify);

		} catch(const std::exception &e) {

			update.next	= time(nullptr) + update.timer;
			cerr << "Agent\tError '" << e.what() << "' loading defaults" << endl;

		}

	}

	Abstract::Agent::Agent(Agent *parent, const pugi::xml_node &node) : Abstract::Agent(parent) {

#ifdef DEBUG
		cout << "Creating " << this->name << endl;
#endif // DEBUG

		this->name = Factory::validate_name(node.attribute("name").as_string());
		this->update.notify = node.attribute("notify").as_bool(this->update.notify);

		this->icon = Udjat::getAttribute(node,"icon").as_string();
		this->label = Udjat::getAttribute(node,"label").as_string();
		this->uri = Udjat::getAttribute(node,"uri").as_string();
		this->summary = Udjat::getAttribute(node,"summary").as_string();

		this->update.timer = node.attribute("update-timer").as_uint(this->update.timer);

		this->update.on_demand = node.attribute("update-on-demand").as_bool(this->update.timer == 0);

		time_t delay = node.attribute("delay-on-startup").as_uint(0);
		if(delay)
			this->update.next = time(nullptr) + delay;


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

	void Abstract::Agent::get(Json::Value &value) {
		chk4refresh();
		this->state->getValue(value);
	}

	Json::Value & Abstract::Agent::setup(const Request &request, Response &response) {

		chk4refresh(true);

		if(update.expires && update.expires > time(nullptr))
			response.setExpirationTimestamp(update.expires);

		if(update.last)
			response.setModificationTimestamp(update.last);

		return response;
	}

	void Abstract::Agent::get(const Request &request, Response &response) {

		auto value = setup(request,response);

		for(auto child : children) {
			child->get(value[child->getName()]);
		}

	}

	Json::Value Abstract::Agent::as_json() {

		Json::Value node;

		chk4refresh(true);

		lock_guard<std::recursive_mutex> lock(guard);

		get(node);
		node["name"] = this->getName();
		node["summary"] = this->summary.c_str();
		node["state"] = this->state->as_json();
		node["label"] = this->label.c_str();
		node["uri"] = this->uri.c_str();
		node["icon"] = this->icon.c_str();

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
