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

 #include <config.h>
 #include <udjat-internals.h>
 #include <sys/time.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	MainLoop::Timer::Timer(const void *i, unsigned long m, const function<bool()> c)
		: id(i), interval(m), call(c) {

		next = this->getCurrentTime() + interval;

	}

	void MainLoop::Timer::reset(unsigned long milliseconds) {
		interval = milliseconds;
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
		unsigned long wait = 60000;

		active.remove_if([this,now,&wait](Timer &timer) {

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
				wait = std::min(wait,timer.interval);

			} else {

				wait = std::min(wait,timer.next-now);

			}

			return false;

		});

		return wait;
	}

	void MainLoop::insert(const void *id, unsigned long interval, const std::function<bool()> call) {

		lock_guard<mutex> lock(guard);
		timers.active.emplace_back(id,interval,call);
		wakeup();

	}

	bool MainLoop::reset(const void *id, unsigned long interval) {

		lock_guard<mutex> lock(guard);
		for(auto timer = timers.active.begin(); timer != timers.active.end(); timer++) {

			if(timer->id == id) {
				timer->reset(interval);
				return true;
			}

		}
		return false;

	}


 }

