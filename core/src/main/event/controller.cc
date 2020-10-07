/**
 * @file
 *
 * @brief Implements the event controller
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <udjat/service.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Abstract::Event::Controller::guard;

	Abstract::Event::Controller::Controller() {

#ifdef DEBUG
		cout << "Event controler starts" << endl;
#endif // DEBUG

		Udjat::Service::insert((void *) this, 5, [this](const time_t now) {

			lock_guard<recursive_mutex> lock(guard);

#ifdef DEBUG
			cout << "Checking pending events (" << actions.size() << ")" << endl;
#endif // DEBUG

			time_t interval = 60;
			time_t next = now + interval;

			actions.remove_if([now,&interval,&next](Action &action) {

				if(!action.event) {
#ifdef DEBUG
					cout << "Event " << *action.event << " ends" << endl;
#endif // DEBUG
					return true;
				}

				if(action.next <= now) {

					action.last = now;
					action.next = now + action.event->retry.interval;
					action.count++;

					try {

						action.call(
							*action.agent,
							*(action.state ? action.state : action.agent->getState().get())
						);

					} catch(const std::exception &e) {

						cerr << "Error \"" << e.what() << "\" activating event " << *action.event << endl;
						return true;

					} catch(...) {

						cerr << "Unexpected error activating event " << *action.event << endl;
						return true;

					}

				}

				next = min(next,action.next);
				interval = min(interval,action.event->retry.interval);

#ifdef DEBUG
				cout << "Interval: " << interval << " On event: " << action.event->retry.interval << endl;
#endif // DEBUG

				return false;

			});

			Service::reset((void *) this, interval, next);
			return true;

		});

	}

	Abstract::Event::Controller::~Controller() {
		Service::remove(this);
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

#ifdef DEBUG
		cout << "Inserting event " << ((void *) event) << endl;
#endif // DEBUG

		actions.emplace_back(event,agent,state,callback);

		cout << "*************** " << this << " " << actions.size() << endl;

		// Reset timer.
		Service::reset(this,1);

	}

	void Abstract::Event::Controller::remove(Abstract::Event *event) {
		lock_guard<recursive_mutex> lock(guard);

#ifdef DEBUG
		cout << "Removing event " << ((void *) event) << endl;
#endif // DEBUG

		actions.remove_if([event](Action &action) {
			return action.event == event;
		});
	}

	void Abstract::Event::Controller::emit(Abstract::Event::Controller::Action &action) noexcept {
	}
}
