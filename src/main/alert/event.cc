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
 #include <ctime>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timestamp.h>

 namespace Udjat {

	std::mutex Alert::Event::guard;

	Alert::Event::Event() {
	}

	Alert::Event::Event(const Quark &name) : Logger(name) {
	}

	Alert::Event::Event(const Abstract::Agent &agent, const Abstract::State &state) : Event((Quark) agent) {

	}

	Alert::Event::~Event() {
		if(parent) {
			parent->remove(this);
		}
	}

	void Alert::Event::disable() {
		alerts.next = 0;
		Alert::Controller::getInstance().remove(this);
	}

	void Alert::Event::success() {
		alerts.success++;
		next();
	}

	void Alert::Event::failed() {
		alerts.failed++;
		next();
	}

	void Alert::Event::checkForSleep(const char *msg) {

		if(parent->retry.restart) {
			restarting = true;
			alerts.next = time(0) + parent->retry.restart;

			info(
				"'{}' {}, stopping",
					getDescription(),
					msg
			);

		} else {
			alerts.next = 0;
			info(
				"'{}' {}, sleeping until {}",
					getDescription(),
					msg,
					TimeStamp(alerts.next).to_string()
			);
		}

	}

	void Alert::Event::next() {

		if(!parent) {
			alerts.next = 0;
			warning("'{}' lost his parent",getDescription());
			return;
		}

		if(alerts.success >= parent->retry.min) {

			checkForSleep("was sucessfull");

		} else if( (alerts.success + alerts.failed) >= parent->retry.max ) {

			checkForSleep("reached the maximum number of emissions");

		} else {

			alerts.next = time(0) + parent->retry.interval;

		}

	}

	void Alert::Event::enqueue(std::shared_ptr<Alert::Event> event) {

		lock_guard<mutex> lock(guard);

		if(!event->parent) {
			event->alerts.next = 0;
			event->error("'{}' lost hist parent",event->getDescription());
			return;
		}

		if(event->running) {
			event->warning(
				"'{}' is active since {}",
					event->getDescription(),
					TimeStamp(event->running).to_string()
			);
			event->alerts.next = time(0) + event->parent->retry.interval;
			return;
		}

		if(event->restarting) {
			event->restarting = false;
			event->info(
				"Restarting '{}'",
					event->getDescription()
			);
			event->reset();
		}

		event->running = time(0);
		event->alerts.next = event->running + event->parent->retry.interval;

		// Is an event restart?
		if(event->restarting) {
			event->restarting = false;
			event->reset();
		}

		// Enqueue alert emission.
		ThreadPool::getInstance().push([event]() {

			try {

				if(event->parent) {

					event->alert(
							event->alerts.success + event->alerts.failed + 1,
							event->parent->retry.max
					);

					event->success();

				} else {
					event->alerts.next = 0;
				}

			} catch(const std::exception &e) {

				event->error("Error '{}' firing event",e.what());
				event->failed();

			} catch(...) {

				event->error("Error '{}' firing event","Unexpected");
				event->failed();

			}

			// Reset 'running' flag.
			{
				lock_guard<mutex> lock(guard);
				event->running = 0;
			}

		});

	}

 }