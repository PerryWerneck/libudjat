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
 #include <private/linux/mainloop.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>
 #include <sys/time.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	MainLoop::Timer::Timer(unsigned long milliseconds) {
		if(!milliseconds) {
			throw system_error(EINVAL,system_category(),"Invalid timer value");
		}
		reset(milliseconds);
	}

	MainLoop::Timer::~Timer() {
		disable();
	}

	void MainLoop::Timer::reset(unsigned long milliseconds) {

		auto saved = next;

		if(milliseconds) {
			this->milliseconds = milliseconds;
			next = getCurrentTime() + milliseconds;
		} else {
			next = getCurrentTime();
		}

		if(next < saved) {
			MainLoop::getInstance().wakeup();
		}

	}

	unsigned long MainLoop::Timer::getCurrentTime() {

		::timeval tv;

		if(gettimeofday(&tv, NULL) < 0) {
			throw system_error(errno,system_category(),"Cant get time of day");
		}

		return (tv.tv_sec * 1000) + (tv.tv_usec /1000);

	}

	bool MainLoop::Timer::enabled() const {
		return MainLoop::getInstance().enabled(this);
	}

	void MainLoop::Timer::enable(unsigned long milliseconds) {
		this->milliseconds = milliseconds;
		enable();
	}

	void MainLoop::Timer::enable() {
		next = getCurrentTime() + milliseconds;
		if(!enabled()) {
			MainLoop::getInstance().push_back(this);
		}
		MainLoop::getInstance().wakeup();
	}

	void MainLoop::Timer::disable() {
		MainLoop::getInstance().remove(this);
	}

	std::string MainLoop::Timer::to_string() const {

		if(!milliseconds) {
			return "none";
		}

		if(!(milliseconds%1000L)) {

			// In seconds.
			unsigned long seconds{milliseconds/1000L};

			if(seconds == 1) {
				return "one second";
			}

			if(!(seconds%3600)) {
				unsigned long hours{seconds/3600};
				if(hours == 1) {
					return "one hour";
				}

				return Logger::Message("{} hours",hours);
			}

			if(!(seconds%60)) {
				unsigned long minutes{seconds/60};
				if(minutes == 1) {
					return "one minute";
				}

				return Logger::Message("{} minutes",minutes);
			}

			return Logger::Message("{} seconds",seconds);
		}

		return Logger::Message( "{} milliseconds", milliseconds);
	}

	unsigned long MainLoop::Timer::activate() noexcept {

		try {

			if(milliseconds) {
				next = getCurrentTime() + milliseconds;
				on_timer();
			} else {
				next = 0;
				on_timer();
				disable();
			}

		} catch(const std::exception &e) {

			Application::error() << "Timer disabled: " << e.what() << endl;
			disable();
			next = 0;

		} catch(...) {

			Application::error() << "Timer disabled: Unexpected error" << endl;
			disable();
			next = 0;

		}

		return next;

	}

	/// @brief Check if timer is expired, activate it if necessary.
	/// @return The updated timer value or '0' if timer was disabled.
	unsigned long check() noexcept;

	/*
	unsigned long MainLoop::Timers::run() noexcept {

		unsigned long now = MainLoop::Timer::getCurrentTime();
		unsigned long next = now + maxwait;

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
	*/


	MainLoop::Timer * MainLoop::TimerFactory(unsigned long interval, const std::function<bool()> call) {

		class CallBackTimer : public Timer {
		private:
			const std::function<bool()> callback;

		protected:
			void on_timer() override {

				bool success = true;

				try {
					success = callback();
				} catch(const std::exception &e) {
					Application::error() << "Timer failed: " << e.what() << endl;
					success = false;
				} catch(...) {
					Application::error() << "Timer failed: Unexpected error"  << endl;
					success = false;
				}

				if(!success) {
					delete this;
				}
			}

		public:
			CallBackTimer(unsigned long milliseconds, const std::function<bool()> c) : Timer(milliseconds), callback(c) {
				debug(__FUNCTION__);
				enable();
			}

		};

		return new CallBackTimer(interval,call);

	}

 }

