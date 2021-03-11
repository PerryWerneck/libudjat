/**
 * @file src/core/agent/controller.cc
 *
 * @brief Implements the root agent.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/tools/threadpool.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::Agent::Controller::Controller() : Worker(I_("agent")) {
	}

	void Abstract::Agent::Controller::set(std::shared_ptr<Abstract::Agent> root) {
		this->root = root;
		this->active = this->root.get();
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::get() {

		if(this->root)
			return this->root;

		throw runtime_error("Agent subsystem is inactive");

	}

	Abstract::Agent::Controller & Abstract::Agent::Controller::getInstance() {
		static Controller controller;
		return controller;
	}

	void Abstract::Agent::Controller::work(const char *path, const Request &request, Response &response) {

		auto agent = find(path);
		if(!agent) {
			throw system_error(ENOENT,system_category(),"Can't find requested agent");
		}

		agent->get(request,response);

	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::find(const char *path) {

		auto root = get();

		if(path && *path)
			return root->find(path);

		return root;

	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::set_root(std::shared_ptr<Abstract::Agent> agent) {
		Abstract::Agent::Controller::getInstance().set(agent);
		return agent;
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::get_root() {
		return Abstract::Agent::Controller::getInstance().get();
	}

}

