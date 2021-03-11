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
 #include <udjat/worker.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	class AgentController : public Worker {
	private:
		std::shared_ptr<Abstract::Agent> root;

	public:

		AgentController() : Worker(I_("agent")) {
			this->active = true;
		}

		void set(std::shared_ptr<Abstract::Agent> root) {
			this->root = root;
			this->active = this->root.get();
		}

		std::shared_ptr<Abstract::Agent> get() {

			if(this->root)
				return this->root;

			throw runtime_error("Agent subsystem is inactive");

		}

		static AgentController & getInstance() {
			static AgentController controller;
			return controller;
		}

		void work(const char *path, const Request &request, Response &response) {

			throw runtime_error("Not implemented");

		}

	};


	void set_root_agent(std::shared_ptr<Abstract::Agent> agent) {
		AgentController::getInstance().set(agent);
	}

	std::shared_ptr<Abstract::Agent> get_root_agent() {
		return AgentController::getInstance().get();
	}

	std::shared_ptr<Abstract::Agent> find_agent(const char *path) {

		auto root = get_root_agent();

		if(path && *path)
			return root->find(path);

		return root;

	}

}

