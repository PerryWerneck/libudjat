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

 namespace Udjat {

	MainLoop::Timer::Timer(void *i, time_t s, const function<bool(const time_t)> c) :
		id(i),running(0),seconds(s),next(time(0)+s),call(c) { }


	MainLoop::Timer::Timer(void *i, const function<bool(const time_t)> c) : Timer(i,1,c) { }

	time_t MainLoop::runTimers(time_t wait) {

		lock_guard<mutex> lock(guard);

		time_t now = time(0);
		timers.remove_if([now,&wait](Timer &timer) {

			// Do I still active? If not return true *only* if not running.
			if(!timer.seconds)
				return (timer.running != 0);

			if(timer.next <= now) {

				// Still have pending events? Ignore it but keep me the list.
				if(timer.running) {
					clog << "MainLoop\tTimer call is taking too long" << endl;
					return false;
				}

				// Timer has expired, update value and enqueue method.
				timer.next = (now + timer.seconds);
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

			wait = std::min(wait, (timer.next - now));

			return false;

		});

		return wait;
	}


 }

