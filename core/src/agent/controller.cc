/**
 * @file src/core/agent/controller.cc
 *
 * @brief Implements the root agent.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/threadpool.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::Agent::Controller::Controller() : Abstract::Agent::Agent() {

		this->name = "";

		cout << "Agent controller " << ((void *) this) << " was created" << endl;

	}

	Abstract::Agent::Controller::~Controller() {

		cout << "Agent controller " << ((void *) this) << " was destroyed" << endl;

	}

	void Abstract::Agent::Controller::append_state(const pugi::xml_node &node) {
		throw std::runtime_error("Can't add states to the root agent.");
	}

	void Abstract::Agent::Controller::refresh() {

		time_t now = time(nullptr);

		foreach([now](std::shared_ptr<Agent> agent) {

#ifdef DEBUG
			cout << "Checking refresh of " << agent->getName() << endl;
#endif // DEBUG

			if(!agent->update.next || agent->update.next > now || agent->update.running)
				return;

#ifdef DEBUG
			cout << "Will refresh " << agent->getName() << endl;
#endif // DEBUG

			agent->update.running = now;

			ThreadPool::getInstance().push([agent]() {

				if(agent->update.timer)
					agent->update.next = time(nullptr) + agent->update.timer;
				else
					agent->update.next = 0;

				try {

#ifdef DEBUG
					cout << "Refreshing " << agent->getName() << endl;
#endif // DEBUG
					agent->refresh();

				} catch(const exception &e) {

					cerr << "Error \"" << e.what() << "\" updating agent " << agent->getName() << endl;

				} catch(...) {

					cerr << "Unexpceted error updating agent " << agent->getName() << endl;

				}

				agent->update.running = 0;

			});

		});


	}


}

