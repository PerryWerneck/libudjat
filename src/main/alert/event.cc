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

	Alert::Event::Event() : name("") {
	}

	Alert::Event::Event(const Quark &n) : name(n) {
	}

	Alert::Event::Event(const Abstract::Agent &agent, const Abstract::State &state) : Event() {

		if( (Quark) state) {
			name = (Quark) state;
		} else {
			name = (Quark) agent;
		}

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

	void Alert::Event::next() {

		if(!parent) {
			alerts.next = 0;
			cout << "Event '" << name << "' lost his parent" << endl;
		}

		if( (alerts.success + alerts.failed) >= parent->retry.limit ) {

			if(parent->retry.restart) {
				restarting = true;
				alerts.next = time(0) + parent->retry.restart;
				parent->info("Alert '{}' reached the maximum number of emissions, sleeping until {}",
								name.c_str(),TimeStamp(alerts.next).to_string());

			} else {
				parent->info("Alert '{}' reached the maximum number of emissions, stopping",name.c_str());
				alerts.next = 0;
			}
		} else {

			alerts.next = time(0) + parent->retry.interval;

		}

	}

	void Alert::Event::enqueue(std::shared_ptr<Alert::Event> event) {

		lock_guard<mutex> lock(guard);

		if(!event->parent) {
			event->alerts.next = 0;
			cout << "Event '" << event->name << "' lost his parent" << endl;
			return;
		}

		if(event->running) {
			event->parent->warning(
				"Event '{}' is active since {}",
					event->name.c_str(),
					TimeStamp(event->running).to_string()
			);
			event->alerts.next = time(0) + event->parent->retry.interval;
			return;
		}

		if(event->restarting) {
			event->restarting = false;
			event->parent->info(
				"Restarting event '{}'",
					event->name.c_str()
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

					event->parent->info(
						"Emitting alert '{}' ({}/{})",
						event->name.c_str(),
						(event->alerts.success + event->alerts.failed + 1),
						event->parent->retry.limit
					);

					event->alert();
					event->success();

				} else {
					event->alerts.next = 0;
				}

			} catch(const std::exception &e) {

				event->parent->error("Error '{}' firing event '{}'",e.what(),event->name.c_str());
				event->failed();

			} catch(...) {

				event->parent->error("Unexpected error firing event '{}'",event->name.c_str());
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
