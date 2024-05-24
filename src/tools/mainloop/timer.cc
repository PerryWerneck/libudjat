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
#ifdef DEBUG
		clog << "MainLoop\t---> Timer " << hex << ((void *) this) << dec
				<< " was deleted and will be disabled" << endl;
#endif // DEBUG
		disable();
	}

	void MainLoop::updated(MainLoop::Timer *, unsigned long from_value, unsigned long to_value) {
		if(to_value < from_value) {
			wakeup();
		}
	}

	bool MainLoop::Timer::set(const unsigned long milliseconds) {

		if(values.interval == milliseconds) {
			return false;
		}

		auto saved = values.activation_time;
		values.interval = milliseconds;
		if(values.interval) {
			values.activation_time = getCurrentTime() + milliseconds;
			MainLoop::getInstance().updated(this,saved,values.activation_time);
		}

		return true;

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
		values.interval = milliseconds;
		enable();
	}

	void MainLoop::Timer::enable() {
		values.activation_time = getCurrentTime() + values.interval;
		if(!enabled()) {
			MainLoop::getInstance().push_back(this);
		}
		MainLoop::getInstance().wakeup();
	}

	void MainLoop::Timer::disable() {
		MainLoop::getInstance().remove(this);
	}

	std::string MainLoop::Timer::to_string() const {

		if(!values.interval) {
			return "none";
		}

		if(!(values.interval%1000L)) {

			// In seconds.
			unsigned long seconds{values.interval/1000L};

			if(seconds == 1) {
				return _( "one second" );
			}

			if(!(seconds%3600)) {
				unsigned long hours{seconds/3600};
				if(hours == 1) {
					return _( "one hour" );
				}

				return Logger::Message(_( "{} hours" ),hours);
			}

			if(!(seconds%60)) {
				unsigned long minutes{seconds/60};
				if(minutes == 1) {
					return _( "one minute" );
				}

				return Logger::Message(_( "{} minutes" ),minutes);
			}

			return Logger::Message(_( " {} seconds" ),seconds);
		}

		return Logger::Message( _( " {} milliseconds" ), values.interval);
	}

	unsigned long MainLoop::Timer::activate() noexcept {

		try {

			if(values.interval) {
				unsigned long rc = values.activation_time = (getCurrentTime() + values.interval);
				on_timer();
				return rc;
			}

			debug("No interval, deactivating");
			on_timer();
			disable();

		} catch(const std::exception &e) {

			Application::error() << "Timer disabled: " << e.what() << endl;
			disable();

		} catch(...) {

			Application::error() << "Timer disabled: Unexpected error" << endl;
			disable();
		}

		return 0;

	}

	/// @brief Check if timer is expired, activate it if necessary.
	/// @return The updated timer value or '0' if timer was disabled.
	unsigned long check() noexcept;

	MainLoop::Timer * MainLoop::Timer::Factory(unsigned long interval, const std::function<bool()> call) {
		return MainLoop::getInstance().TimerFactory(interval, call);
	}

	MainLoop::Timer * MainLoop::TimerFactory(unsigned long interval, const std::function<bool()> call) {

		class CallBackTimer : public Timer {
		private:
			const std::function<bool()> callback;

		protected:
			void on_timer() override {

#ifdef DEBUG
				clog << "MainLoop\t---> Activating timer " << hex << ((void *) this) << dec
						<< " " << this->to_string() << endl;
#endif // DEBUG

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
#ifdef DEBUG
					clog << "MainLoop\t---> Factored timer " << hex << ((void *) this) << dec
							<< " has failed and will be deleted" << endl;
#endif // DEBUG
					delete this;
					return;
				}

#ifdef DEBUG
				clog << "MainLoop\t---> Factored timer " << hex << ((void *) this) << dec
						<< " still active" << this->to_string() << endl;
#endif // DEBUG
			}

		public:
			CallBackTimer(unsigned long milliseconds, const std::function<bool()> c) : Timer(milliseconds), callback(c) {
#ifdef DEBUG
			clog << "MainLoop\t---> Factoring timer " << hex << ((void *) this) << dec
					<< " " << this->to_string() << endl;
#endif // DEBUG
				enable();
			}

		};

		return new CallBackTimer(interval,call);

	}

 }

