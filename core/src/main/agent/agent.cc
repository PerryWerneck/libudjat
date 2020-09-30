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

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::recursive_mutex Abstract::Agent::guard;

	static std::shared_ptr<Abstract::State> getDefaultState() {

		std::shared_ptr<Abstract::State> state;

		if(!state) {
			state = std::make_shared<Abstract::State>(Abstract::State::unimportant,"Undefined");
		}

		return state;

	}

	Abstract::Agent::Agent() {
		this->state = getDefaultState();
	}

	Abstract::Agent::Agent(Agent *parent, const pugi::xml_node &node) : Abstract::Agent() {

		this->name = Factory::validate_name(node.attribute("name").as_string());

#ifdef DEBUG
		cout << "Creating " << this->name << endl;
#endif // DEBUG

		this->parent = parent;

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

	void Abstract::Agent::refresh() {
	}

	void Abstract::Agent::revalidate() {

#ifdef DEBUG
		cerr << "Revalidating \"" << this->name << "\"" << endl;
#endif // DEBUG

		// Compute new state
		try {

			auto new_state = find_state();

			// Does any children has worst state? if yes; use-it.
			{
				lock_guard<std::recursive_mutex> lock(guard);
				for(auto child : children) {

					auto child_state = child->find_state();

					if(child_state && (!new_state || child_state->getLevel() > new_state->getLevel())) {
						new_state = child_state;
					}

				}

			}

			if(new_state != this->state) {

				if(this->state) {
					this->state->deactivate(*this);
				}

				this->state = new_state;

				if(this->state) {
					cout << "State of " << *this << " is now " << this->state << endl;
					this->state->activate(*this);
				} else {
					cout << "State of " << *this << " is now undefined" << endl;
				}

			}

		} catch(const exception &e) {

			cerr << "Çan't update state of \"" << this->name << "\": " << e.what() << endl;
			this->state = make_shared<State>(State::critical,e.what());

		} catch(...) {

			cerr << "Çan't update state of \"" << this->name << "\": Unexpected error" << endl;
			this->state = make_shared<State>(State::critical,"Unexpected error");

		}

		if(parent)
			parent->revalidate();

	}

	void Abstract::Agent::start() {

		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {
			child->start();
		}

		// Update state.
		this->state = find_state();

		// Check for children state
		{
			lock_guard<std::recursive_mutex> lock(guard);
			for(auto child : children) {

				auto child_state = child->find_state();

				if(child_state && (!this->state || child_state->getLevel() > this->state->getLevel())) {
					this->state = child_state;
				}

			}

		}

		if(this->state && this->name) {
			clog << this->name << " starts with state \"" << this->state << "\"" << endl;
		}

	}

	void Abstract::Agent::stop() {

		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : children) {
			child->stop();
		}
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::find(const char *name) {

		lock_guard<std::recursive_mutex> lock(guard);

#ifdef DEBUG
		cout << "Searching for agent " << name << endl;
#endif // DEBUG

		for(auto child : children) {
			if(!strcasecmp(child->name.c_str(),name)) {
				return child;
			}
		}

		throw runtime_error("Can't find agent");

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

	void Abstract::Agent::chk4refresh() {

		// Return if update is running.
		if(update.running)
			return;

		if(!update.on_demand) {

			// It's not on-demand, check for timer

			// Return if have update time in the future.
			if(update.next && update.next > time(nullptr))
				return;

		}

		update.running = time(nullptr);

		if(update.timer)
			update.next = update.running + update.timer;
		else
			update.next = 0;

		try {

			refresh();

			update.last = time(nullptr);

		} catch(...) {

			update.running = 0;
			throw;

		}

		update.running = 0;

	}

	Json::Value Abstract::Agent::as_json() {

		Json::Value node;

		lock_guard<std::recursive_mutex> lock(guard);

		if(children.empty()) {
			get(node);
		} else {

			for(auto child : children) {
				node[child->getName()] = child->as_json();
			}

		}

		return node;

		/*
		Json::Value value;

		lock_guard<std::recursive_mutex> lock(guard);

		if(children.empty()) {

			// Single value.
			this->get(value[this->name.c_str()]);

		} else {

			// Multiple values.
			for(auto child : children) {
				value[this->name.c_str()] = child->as_json();
			}

		}
		return value;
		*/


	}

	void Abstract::Agent::append_state(const pugi::xml_node &node) {
		string str("Can't append state on agent \"");
		str += this->name.c_str();
		str += "\": It's a non-typed agent";
		throw runtime_error(str);
	}

	std::shared_ptr<Abstract::State> Abstract::Agent::find_state() const {
		// TODO: Search for childs.
		static std::shared_ptr<Abstract::State> st;
		return st;
	}

}
