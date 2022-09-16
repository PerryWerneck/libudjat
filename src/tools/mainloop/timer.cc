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
 #include <udjat/tools/timer.h>
 #include <sys/time.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	MainLoop::Timer::~Timer() {
		disable();
	}

	void MainLoop::Timer::reset(unsigned long milliseconds) {

		if(!milliseconds) {
			throw system_error(EINVAL,system_category(),"Child agent cant be set as root");
		}

		this->milliseconds = milliseconds;
	}

	unsigned long MainLoop::Timer::getCurrentTime() {

		::timeval tv;

		if(gettimeofday(&tv, NULL) < 0) {
			throw system_error(errno,system_category(),"Cant get time of day");
		}

		return (tv.tv_sec * 1000) + (tv.tv_usec /1000);

	}

	void MainLoop::Timer::enable() {

		MainLoop &mainloop{MainLoop::getInstance()};

		next = getCurrentTime() + milliseconds;

		{
			lock_guard<mutex> lock(mainloop.guard);
			mainloop.timers.enabled.push_back(this);
		}

		mainloop.wakeup();
	}

	void MainLoop::Timer::disable() {

		MainLoop &mainloop{MainLoop::getInstance()};

		{
			lock_guard<mutex> lock(mainloop.guard);
			mainloop.timers.enabled.remove(this);
		}

		mainloop.wakeup();
	}

	unsigned long MainLoop::Timers::run() noexcept {

		unsigned long now = MainLoop::Timer::getCurrentTime();
		unsigned long next = now + 60000;

		//
		// Get expired timers.
		//
		std::list<Timer *> expired;
		{
			lock_guard<mutex> lock(guard);
			for(Timer *timer : enabled) {
				if(timer->next <= now) {
					expired.push_back(timer);
				} else {
					next = std::min(next,timer->next);
				}
			}

		}

		//
		// Emit timer events.
		//
		for(auto timer : expired) {

			try {

				if(timer->milliseconds) {
					timer->next = now + timer->milliseconds;
					next = std::min(next,timer->next);
					timer->on_timer();
				} else {
					timer->disable();
				}

			} catch(const std::exception &e) {

				Application::error() << "Timer disabled: " << e.what() << endl;
				timer->disable();

			} catch(...) {

				Application::error() << "Timer disabled: Unexpected error" << endl;
				timer->disable();

			}

		}

		return next - now;

	}

	/*
	MainLoop::Timer::Timer(const void *i, unsigned long m)
		: interval(m), id(i) {

		next = this->getCurrentTime() + interval;

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

			class CallBackTimer : public Timer {
			private:
				const std::function<bool()> callback;

			public:
				CallBackTimer(const void *id, unsigned long milliseconds, const std::function<bool()> c) : Timer(id,milliseconds), callback(c) {
				}

				bool call() const override {
					return callback();
				}

			};

			timers.active.push_back(make_shared<CallBackTimer>(id,interval,call));

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
	*/

 }

