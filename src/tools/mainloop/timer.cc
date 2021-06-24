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
 #include <sys/time.h>

 namespace Udjat {

	MainLoop::Timer::Timer(const void *i, unsigned long m, const function<bool()> c)
		: id(i), interval(m), call(c) {

		next = this->getCurrentTime() + interval;

	}

	unsigned long MainLoop::Timer::getCurrentTime() {

		::timeval tv;

		if(gettimeofday(&tv, NULL) < 0) {
			throw system_error(errno,system_category(),"Cant get time of day");
		}

		return (tv.tv_sec * 1000) + (tv.tv_usec /1000);

	}

	unsigned long MainLoop::Timers::run() noexcept {

		lock_guard<mutex> lock(guard);

		unsigned long now = MainLoop::Timer::getCurrentTime();
		unsigned long next = now + 60000;

		active.remove_if([this,now,&next](Timer &timer) {

			// No interval; looks like the timer was deactivated.
			// Do I still active? Return true *only* if not running.
			if(!timer.interval) {
				return !timer.running;
			}

			if(timer.next <= now) {

				try {

					if(!timer.call()) {
						return true;
					}

				} catch(const std::exception &e) {

					cerr << "MainLoop\tTimer error '" << e.what() << "'" << endl;

				} catch(...) {

					cerr << "MainLoop\tUnexpected error on timer" << endl;

				}

				if(!timer.interval) {
					return true;
				}

				timer.next = now + timer.interval;

			}

			next = std::min(next,timer.next);

			return false;
		});

		return next - now;
	}

	void MainLoop::insert(const void *id, unsigned long interval, const std::function<bool()> call) {

		lock_guard<mutex> lock(guard);
		timers.active.emplace_back(id,interval,call);
		wakeup();

	}

	/*
	MainLoop::Timer::Timer(const void *i, time_t s, const function<bool(const time_t)> c) :
		id(i),running(0),seconds(s),next(time(0)+s),call(c) { }


	time_t MainLoop::reset(const void *id, time_t seconds, time_t time) {

		lock_guard<mutex> lock(guard);
		for(auto timer = timers.active.begin(); timer != timers.active.end(); timer++) {

			if(timer->id == id && timer->seconds) {

				if(seconds > 0)
					timer->seconds = seconds;

				if(!time)
					time = ::time(0) + timer->seconds;

				time_t current = timer->next;
				timer->next = time;

				// If the new timer is lower than the last one wake up main loop to adjust.
				if(timer->next <= timers.next) {
					wakeup();
				}

				return current;
			}

		}
		return 0;
	}

	MainLoop::Timer::Timer(const void *i, const function<bool(const time_t)> c) : Timer(i,1,c) { }

	time_t MainLoop::Timers::run() noexcept {

		lock_guard<mutex> lock(guard);

		time_t now = time(0);
		next = now + def; // Reset to default value.

		active.remove_if([this,now](Timer &timer) {

			// No seconds or timestamp; looks like the timer was deactivated.
			// Do I still active? Return true *only* if not running.
			if(!(timer.seconds && timer.next))
				return (timer.running != 0);

			if(timer.next > now) {

				// Timer is not expired.
				next = std::min(next,timer.next);

			} else {

				// Timer has expired.

				// Still have pending events? Ignore it but keep me the list.
				if(timer.running) {
					clog << "MainLoop\tTimer call is taking too long" << endl;
					timer.next = (now + timer.seconds);
					next = std::min(next,timer.next);
					return false;
				}

				// Timer has expired, update value and enqueue method.
				timer.next = (now + timer.seconds);
				next = std::min(next,timer.next);
				timer.running = now;

				try {

					if(timer.seconds && !timer.call(now))
						timer.seconds = 0;

				} catch(const exception &e) {

					cerr << "MainLoop\tTimer error '" << e.what() << "'" << endl;

				} catch(...) {

					cerr << "MainLoop\tUnexpected error on timer" << endl;

				}

				timer.running = 0;

			}

			return false;

		});

		return next - now;
	}
	*/

 }

