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

		Udjat::Service::insert((void *) this, 600, [this](const time_t now) {

			lock_guard<recursive_mutex> lock(guard);

			time_t interval = 60;
			time_t next = now + interval;

			actions.remove_if([now,&interval,&next](Action &action) {

				if(!action.event) {
					return true;
				}

				if(++action.count > action.event->retry.limit) {
					return true;
				}

				if(action.next <= now) {

					action.last = now;
					action.next = now + action.event->retry.interval;

					try {

						action.call(*action.agent, action.getState());

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

		actions.emplace_back(event,agent,state,event->retry.first,callback);

		// Reset timer.
		Service::reset(this,0,time(nullptr)+event->retry.first);

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

	void Abstract::Event::Controller::forEach(const std::function<void(const Abstract::Event &event, const Abstract::Agent &agent, const Abstract::State &state, time_t last, time_t next, size_t count)> call) {

		lock_guard<recursive_mutex> lock(guard);

		for(auto action : actions) {
			if(action.event)
				call(*action.event, *action.agent, action.getState(), action.last, action.next, action.count);
		}

	}

	void Abstract::Event::forEach(const std::function<void(const Abstract::Event &event, const Abstract::Agent &agent, const Abstract::State &state, time_t last, time_t next, size_t count)> call) {
		Controller::getInstance().forEach(call);
	}


}
