/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include "private.h"
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/factory.h>

 namespace Udjat {

	mutex Alert::Controller::guard;

	Alert::Controller::Controller() : Worker(Quark::getFromStatic("alerts")) {

		static const Udjat::ModuleInfo info{
			PACKAGE_NAME,								// The module name.
			"Alert Controller",							// The module description.
			PACKAGE_VERSION "." PACKAGE_RELEASE,		// The module version.
#ifdef PACKAGE_URL
			PACKAGE_URL,
#else
			"",
#endif // PACKAGE_URL
#ifdef PACKAGE_BUG_REPORT
			PACKAGE_BUG_REPORT
#else
			""
#endif // PACKAGE_BUG_REPORT
		};

		Worker::info = &info;

		MainLoop::getInstance().insert(this, 600, [this](const time_t now){
			ThreadPool::getInstance().push([this,now]() {
				onTimer(now);
			});
			return true;
		});


	}

	Alert::Controller::~Controller() {
		MainLoop::getInstance().remove(this);
	}

	void Alert::Controller::onTimer(time_t now) noexcept {

		lock_guard<mutex> lock(guard);
#ifdef DEBUG
		cout << "Checking for events" << endl;
#endif // DEBUG

		time_t timer_value = 600;
		events.remove_if([now,&timer_value](std::shared_ptr<Alert::Event> event){

			if(!event->next)
				return true;

			if(event->next < now) {

				timer_value = std::min(timer_value,event->next);

			} else {

				// Is a restart?
				if(event->restarting) {
					event->alert->warning("{}","Restarting event");
					event->current = 0;
					event->restarting = false;
				}

				// Get interval for next try.
				time_t interval = 0;
				if(event->current > event->alert->retry.limit) {
					interval = event->alert->retry.restart;
					event->restarting = true;
				} else {
					interval = event->alert->retry.interval;
				}

				if(interval) {
					event->next = now + interval;
				} else {
					event->next = 0;
				}

				event->last = now;
				event->current++;

				// Fire event.
				ThreadPool::getInstance().push([event]() {

					try {

						event->fire();
						if(event->alert->disable_on_success) {
							event->disable();
						}

					} catch(const std::exception &e) {

						event->alert->error("Error '{}' firing event",e.what());
						if(event->alert->disable_when_failed) {
							event->disable();
						}

					} catch(...) {

						event->alert->error("Error '{}' firing event","unexpected");
						if(event->alert->disable_when_failed) {
							event->disable();
						}

					}
				});

			}

			return event->next == 0;

		});

#ifdef DEBUG
		cout << "Event timer set to " << to_string(timer_value) << endl;
#endif // DEBUG

		MainLoop::getInstance().reset(this,timer_value,now+timer_value);

	}

	Alert::Controller & Alert::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	void Alert::Controller::work(const Request &request, Response &response) const {


		throw runtime_error("Not implemented");
	}

	void Alert::Controller::insert(Alert *alert, std::shared_ptr<Alert::Event> event) {
		lock_guard<mutex> lock(guard);

		event->alert = alert;
		alert->active = true;
		event->next = (time(nullptr) + alert->retry.start);

		events.push_back(event);
		MainLoop::getInstance().reset(this);
	}

	void Alert::Controller::remove(const Alert *alert) {
		lock_guard<mutex> lock(guard);
		events.remove_if([alert](std::shared_ptr<Alert::Event> event){
			return event->alert == alert;
		});
	}

	void Alert::Controller::remove(const Alert::Event *ev) {
		lock_guard<mutex> lock(guard);
		events.remove_if([ev](std::shared_ptr<Alert::Event> event){
			return event.get() == ev;
		});
	}

	string Alert::Controller::getType(const pugi::xml_node &node) {

		string type =
			Attribute(node,"type",false)
				.as_string(
					Config::Value<string>("alert-default","type","default").c_str()
				);

		return type;

	}



 }
