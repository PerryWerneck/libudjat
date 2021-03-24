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

#ifdef HAVE_EVENTFD
 #include <sys/eventfd.h>
#endif // HAVE_EVENTFD

 namespace Udjat {

	MainLoop & MainLoop::getInstance() {
		static MainLoop mainloop;
		return mainloop;
	}

	MainLoop::MainLoop() {

#ifdef HAVE_EVENTFD
		efd = eventfd(0,0);
		if(efd < 0)
			throw system_error(errno,system_category(),"eventfd() has failed");
#endif // HAVE_EVENTFD

	}

	MainLoop::~MainLoop() {

		enabled = false;
		wakeup();

		{
			lock_guard<mutex> lock(guard);
#ifdef HAVE_EVENTFD
			::close(efd);
#endif // HAVE_EVENTFD
		}

	}

	void MainLoop::remove(void *id) {

		lock_guard<mutex> lock(guard);

		//
		// We can't simple remove the handlers; they can be waiting for a slot to run.
		//
		for(auto timer : timers) {
			if(timer.id == id) {
				timer.seconds = 0;	// When set to '0' the timer will be removed when possible.
			}
		}

		for(auto handler = handlers.begin(); handler != handlers.end(); handler++) {
			if(handler->id == id) {
				handler->fd = -1;	// When set to '-1' the handle will be removed when possible.
			}
		}

		wakeup();
	}

	void MainLoop::insert(void *id, int fd, const Event event, const function<bool(const Event event)> call) {
		lock_guard<mutex> lock(guard);
		handlers.emplace_back(id,fd,event,call);
		wakeup();
	}

	void MainLoop::insert(void *id, time_t seconds, const function<bool(const time_t)> call) {
		lock_guard<mutex> lock(guard);
		timers.emplace_back(id,seconds,call);
		wakeup();
	}

	void MainLoop::insert(void *id, const std::function<bool(const time_t)> call) {

		lock_guard<mutex> lock(guard);

		Timer tm(id,call);
		tm.next = time(nullptr);
		timers.push_back(tm);
		wakeup();

	}

	time_t MainLoop::reset(void *id, time_t seconds, time_t time) {

		lock_guard<mutex> lock(guard);
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


 }

