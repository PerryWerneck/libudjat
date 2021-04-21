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

		// Clear events and wait for all tasks to complete.
		events.clear();
		ThreadPool::getInstance().wait();

	}

	void Alert::Controller::onTimer(time_t now) noexcept {

		lock_guard<mutex> lock(guard);

		time_t next = now+600;
		events.remove_if([now,&next](std::shared_ptr<Alert::Event> event){

			// No next? That's unexpected; ignore.
			if(!event->next) {
				cerr << "Unexpected event '" << event->name << "' whit empty timer, disabling it" << endl;
				return true;
			}

			if(!event->alert) {
				clog << "Event '" << event->name << "' has lost his alert, disabling it" << endl;
				return true;
			}

			if(event->next < now) {

				next = std::min(next,event->next);

			} else {

				// Enqueue event.
				Event::enqueue((event));

				if(event->next) {
					next = std::min(next,event->next);
				}

			}

			return event->next == 0;

		});

		MainLoop::getInstance().reset(this,1,next);

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

		alert->insert(event.get());

		if(alert->retry.start) {
			event->next = (time(nullptr) + alert->retry.start);
		} else {
			Event::enqueue(event);
		}

		{
			lock_guard<mutex> lock(guard);
			events.push_back(event);
			MainLoop::getInstance().reset(this,alert->retry.start,event->next);
		}

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
