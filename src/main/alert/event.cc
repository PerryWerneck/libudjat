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

 namespace Udjat {

	Alert::Event::Event(const Quark &n) : name(n) {
	}

	Alert::Event::~Event() {
		if(alert) {
			alert->remove(this);
		}
	}

	void Alert::Event::disable() {
		next = 0;
		Alert::Controller::getInstance().remove(this);
	}

	void Alert::Event::enqueue(std::shared_ptr<Alert::Event> event) {

		event->last = time(0);
		event->current++;

		// Is an event restart?
		if(event->restarting) {
			event->alert->warning("{}","Restarting event");
			event->current = 1;
			event->restarting = false;
		}

		// Set timer for next event
		{
			if(event->current >= event->alert->retry.limit) {

				// More than the maximum tries, reset.
				if(event->alert->retry.restart) {
					event->next = event->last + event->alert->retry.restart;
					event->restarting = true;
				} else {
					event->next = 0;
				}

			} else {

				event->next = event->last + event->alert->retry.interval;

			}

		}

		// Fire event.
		ThreadPool::getInstance().push([event]() {

			try {

				if(event->alert) {

					event->alert->info(
						"Firing event '{}' ({}/{})",
						event->name.c_str(),
						event->current,
						event->alert->retry.limit
					);

					event->fire();
					if(event->alert->disable_on_success) {
						event->disable();
						event->alert->info("Event '{}' complete",event->name.c_str());
					}

				} else {
					event->next = 0;
				}

			} catch(const std::exception &e) {

				event->alert->error("Error '{}' firing event '{}'",e.what(),event->name.c_str());
				if(event->alert->disable_when_failed) {
					event->disable();
				}

			} catch(...) {

				event->alert->error("Unexpected erro firing event '{}'",event->name.c_str());
				if(event->alert->disable_when_failed) {
					event->disable();
				}

			}
		});

	}

 }
