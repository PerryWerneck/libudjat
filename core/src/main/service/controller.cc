
#include <config.h>
#include "private.h"

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef HAVE_EVENTFD
	#include <sys/eventfd.h>
#endif // HAVE_EVENTFD

namespace Udjat {

	Service::Controller::Controller() {
#ifdef HAVE_EVENTFD
		efd = eventfd(0,0);
		if(efd < 0) {
			throw system_error(errno,system_category(),"eventfd() has failed");
		}
#endif // HAVE_EVENTFD
	}

	Service::Controller::Timer::Timer(void *i, time_t s, const function<bool(const time_t)> c)
		: id(i), seconds(s), next(time(nullptr)+s), running(0), call(c) {
	}

	Service::Controller::Handle::Handle(void *i, int f, const Event e, const function<bool(const Event event)> c)
		: id(i), fd(f), events(e), running(0), call(c) {
	}

	Service::Controller::~Controller() {

		enabled = false;
		wakeup();

		{

			lock_guard<recursive_mutex> lock(guard);

#ifdef HAVE_EVENTFD
			close(efd);
#endif // HAVE_EVENTFD

			for(auto module : modules) {
				try {
					if(module->active) {
						module->active = false;
						module->stop();
					}
				} catch(const std::exception &e) {
					cerr << e.what() << endl;
				}
			}

		}

	}

	Service::Controller & Service::Controller::getInstance() {
		static Service::Controller controller;
		return controller;
	}

	void Service::Controller::insert(Service::Module *module) {
		lock_guard<recursive_mutex> lock(guard);
		modules.emplace_back(module);
		wakeup();
	}

	void Service::Controller::remove(void *id) {

		lock_guard<recursive_mutex> lock(guard);

		modules.remove_if([id](Module *module){

			if(id != ((void *) module))
				return false;

			try {
				if(module->active) {
					module->active = false;
					module->stop();
				}
			} catch(const std::exception &e) {
				cerr << e.what() << endl;
			}

			return true;

		});

		//
		// We can't simple remove the handlers; they can be waiting for a slot to run.
		//
		for(auto timer : timers) {
			if(timer.id == id) {
				timer.seconds = 0;	// When set to '0' the timer will be removed when possible.
			}
		}

		for(auto handler : handlers) {
			if(handler.id == id) {
				handler.fd = -1;	// When set to '-1' the handle will be removed when possible.
			}
		}

		wakeup();
	}

	/// @brief Run service main loop.
	void Service::run() noexcept {
		Service::Controller::getInstance().run();
	}

	void Service::Controller::insert(void *id, int fd, const Event event, const function<bool(const Event event)> call) {
		lock_guard<recursive_mutex> lock(guard);
		handlers.emplace_back(id,fd,event,call);
		wakeup();
	}

	void Service::Controller::insert(void *id, int seconds, const function<bool(const time_t)> call) {
		lock_guard<recursive_mutex> lock(guard);
		timers.emplace_back(id,seconds,call);
		wakeup();
	}

	time_t Service::Controller::reset(void *id, int seconds, time_t time) {

		lock_guard<recursive_mutex> lock(guard);
		for(auto timer = timers.begin(); timer != timers.end(); timer++) {

			if(timer->id == id && timer->seconds) {

				if(seconds > 0)
					timer->seconds = seconds;

				if(!time) {
					time = ::time(nullptr)+timer->seconds;
				}

				time_t current = timer->next;
				timer->next = time;

				// If the new timer is lower than the last one wake up main loop to adjust.
				if(timer->next < current) {
					wakeup();
				}

				return current;
			}

		}
		return 0;
	}

	void Service::reset(void *id, int seconds, time_t value) {
		Service::Controller::getInstance().reset(id,seconds,value);
	}

	/// @brief Insert socket/file in the list of event sources.
	void Service::insert(void *id, int fd, const Event event, const function<bool(const Event event)> call) {
		Service::Controller::getInstance().insert(id,fd,event,call);
	}

	/// @brief Insert timer in the list of event sources.
	void Service::insert(void *id, int seconds, const function<bool(const time_t)> call) {
		Service::Controller::getInstance().insert(id,seconds,call);
	}

	/// @brief Remove socket/file/timer from the list of event sources.
	void Service::remove(void *id) {
		Service::Controller::getInstance().remove(id);
	}

}
