
#include <config.h>
#include "private.h"

namespace Udjat {

	Service::Controller::Controller() {

	}

	Service::Controller::~Controller() {

		for(auto module : modules) {
			try {
				module->stop();
			} catch(const std::exception &e) {
				cerr << e.what() << endl;
			}
		}

	}

	Service::Controller & Service::Controller::getInstance() {
		Service::Controller controller;
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
	void Service::run() {
		Service::Controller::getInstance().run();
	}

	/// @brief Insert socket/file in the list of event sources.
	void Service::insert(void *id, int fd, const Event event, const function<void(const Event event)> call) {
		Service::Controller::getInstance().insert(id,fd,event,call);
	}

	/// @brief Insert timer in the list of event sources.
	void Service::insert(void *id, int seconds, const function<void(const time_t)> call) {
		Service::Controller::getInstance().insert(id,seconds,call);
	}

	/// @brief Remove socket/file/timer from the list of event sources.
	void Service::remove(void *id) {
		Service::Controller::getInstance().remove(id);
	}

}
