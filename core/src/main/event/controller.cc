/**
 * @file
 *
 * @brief Implements the event controller
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Abstract::Event::Controller::guard;

	Abstract::Event::Controller::Controller() {
	}

	Abstract::Event::Controller::~Controller() {
	}

	Abstract::Event::Controller & Abstract::Event::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;
	}

	void Abstract::Event::Controller::insert(Abstract::Event *event, const Abstract::Agent *agent, const Abstract::State *state, const std::function<void(const Abstract::Agent &agent, const Abstract::State &state)> callback) {
		lock_guard<recursive_mutex> lock(guard);

		// Remove other actions from the same event.
		actions.remove_if([event](Action &action) {
			return action.event == event;
		});

		// Insert new action.
		actions.emplace_back(event,agent,state,callback);
	}

	void Abstract::Event::Controller::remove(Abstract::Event *event) {
		lock_guard<recursive_mutex> lock(guard);

		actions.remove_if([event](Action &action) {
			return action.event == event;
		});
	}

	void Abstract::Event::Controller::emit(Abstract::Event::Controller::Action &action) noexcept {
	}
}
