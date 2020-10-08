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

	static std::shared_ptr<Abstract::Agent> root_agent;

	void set_root_agent(std::shared_ptr<Abstract::Agent> agent) {
		root_agent = agent;
	}

	std::shared_ptr<Abstract::Agent> get_root_agent() {

		if(!root_agent)
			throw runtime_error("Agent controller is non existent or inactive");

		return root_agent;
	}

	std::shared_ptr<Abstract::Agent> find_agent(const char *path) {

		auto root = get_root_agent();

		if(path && *path)
			return root_agent->find(path);

		return root_agent;

	}

}

