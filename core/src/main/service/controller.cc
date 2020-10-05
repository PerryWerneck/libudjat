
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
		: id(i), seconds(s), next(time(nullptr)+s), call(c) {
	}

	Service::Controller::Handle::Handle(void *i, int f, const Event e, const function<bool(const Event event)> c)
		: id(i), fd(f), events(e), call(c) {
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

		timers.remove_if([id](Timer &t){
			return t.id == id;
		});

		handlers.remove_if([id](Handle &h){
			return h.id == id;
		});

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
