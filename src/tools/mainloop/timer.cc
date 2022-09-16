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
 #include <private/mainloop.h>
 #include <udjat/tools/application.h>
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

		unsigned long now = MainLoop::Timer::getCurrentTime();
		unsigned long next = now + 60000;

		//
		// Get expired timers.
		//
		std::list<std::shared_ptr<Timer>> timers;
		{
			lock_guard<mutex> lock(guard);

			active.remove_if([this,now,&next,&timers](auto timer) {

				if(!timer->interval) {
					return true;
				}

				if(timer->next <= now) {
					timers.push_back(timer);
				} else {
					next = std::min(next,timer->next);
				}

				return false;
			});

		}

		//
		// Call expired timers
		//
		for(auto timer : timers) {

			try {

				if(!timer->call()) {
					timer->interval = 0;
				}

			} catch(const std::exception &e) {

				Application::error() << "Timer error '" << e.what() << "'" << endl;
				timer->interval = 0;

			} catch(...) {

				Application::error() << "Unexpected error on timer" << endl;
				timer->interval = 0;

			}

			if(timer->interval) {
				timer->next = now + timer->interval;
				next = std::min(next,timer->next);
			} else {
				lock_guard<mutex> lock(guard);
				active.remove(timer);
			}

		}

		return next - now;
	}

	void MainLoop::insert(const void *id, unsigned long interval, const std::function<bool()> call) {

		{
			lock_guard<mutex> lock(guard);
			timers.active.push_back(make_shared<Timer>(id,interval,call));
		}
		wakeup();

	}

	bool MainLoop::reset(const void *id, unsigned long interval) {

		{
			lock_guard<mutex> lock(guard);
			for(auto timer : timers.active) {
				if(timer->id == id) {
					timer->reset(interval);
					return true;
				}
			}
		}

		wakeup();

		return false;

	}


 }

