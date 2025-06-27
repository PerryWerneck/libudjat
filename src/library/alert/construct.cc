/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

 #undef LOG_DOMAIN
 #define LOG_DOMAIN "alert"

 #include <udjat/defs.h>
 #include <udjat/alert.h>
 #include <udjat/tools/logger.h>

 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/timer.h>
 #include <functional>

 namespace Udjat {

	class Alert::Controller : private Container<Alert> {
	private:

		Timer *timer;

		static bool on_timer() noexcept {
			ThreadPool::getInstance().push([](){
				getInstance().wakeup();
			});
			return true;
		}

		Controller() : timer{Timer::Factory(60000,[](){return on_timer();})} {
			Logger::String{"Alert controller initialized"}.info();
		}

		~Controller() {
			delete timer;
			Logger::String{"Alert controller finalized"}.info();
		}

	public:
		static Controller & getInstance() {
			static Controller instance;
			return instance;
		}

		inline void add(Alert * alert) noexcept {
			Container<Alert>::add(alert);
			wakeup();
		}

		inline void remove(Alert * alert) noexcept {
			Container<Alert>::remove(alert);
			wakeup();
		}

		/// @brief Check for active alerts, emit if necessary, reset timer for next alert.
		void wakeup() {

			std::lock_guard<std::mutex> lock(*this);
			debug("Waking up alert controller");

			time_t now = time(0);
			time_t next = now + 600;

			for(Alert *alert : *this) {

				if(!alert->activation.next) {
					continue;
				}

				if(alert->activation.next <= now) {	

					// Emit alert.
					alert->activation.running = true;
					alert->activation.next = 0;
					ThreadPool::getInstance().push([alert](){
						try {
							int result = alert->emit();
							if(result) {
								alert->failed(String{"Alert emission failed with rc=",result}.c_str());
							} else {
								alert->success();
							}
						} catch(const std::exception &e) {
							alert->failed(e.what());
						} catch(...) {
							alert->failed("Unexpected error");
						}
						alert->activation.running = false;
						getInstance().wakeup();
					});

				} else {
					next = std::min(next,alert->activation.next);
				}

			}

			debug("Next alert in ",next - now," seconds at ",TimeStamp(next).to_string().c_str());
			timer->set((next - now) * 1000);

		}

	};

	Alert::Alert(const char *n) : Activatable{n} {
		Controller::getInstance().add(this);
	}

	Alert::~Alert() {
		Controller::getInstance().remove(this);
	}

	Alert::Alert(const XML::Node &node) : Alert(String{node,"name","unnamed"}.as_quark()) {

		// Seconds to wait before first activation.
		timers.start = XML::AttributeFactory(node,"delay-before-start").as_uint(timers.start);

		// Seconds to wait on every try.
		timers.interval = XML::AttributeFactory(node,"delay-before-retry").as_uint(timers.interval);

		// How many success emissions after deactivation or sleep?
		retry.min = XML::AttributeFactory(node,"min-retries").as_uint(retry.min);

		// How many retries (success+fails) after deactivation or sleep?
		retry.max = XML::AttributeFactory(node,"max-retries").as_uint(retry.max);

		// How many seconds to restart when failed?
		restart.failed = XML::AttributeFactory(node,"restart-when-failed").as_uint(restart.failed);

		// How many seconds to restart when suceeded?
		restart.success = XML::AttributeFactory(node,"restart-when-succeeded").as_uint(restart.success);

		if(XML::AttributeFactory(node,"enabled").as_bool(false)) {
			activate();
		}

	}

	void Alert::reset(time_t next) noexcept {
		activation.suceeded = 0;
		activation.failed = 0;
		activation.next = next;
	}

	bool Alert::active() const noexcept {
		return activation.next != 0 || activation.running;
	}

	bool Alert::activate() noexcept {
		if(active()) {
			errno = EALREADY;
			return false;
		}
		reset(time(0)+timers.start);
		Logger::String{
			"Alert activation scheduled to ",
			TimeStamp(activation.next).to_string().c_str()
		}.info(name());
		Controller::getInstance().wakeup();
		return true;
	}

	void Alert::activate(time_t next) noexcept {
		if(!active()) {
			reset(next);
			Logger::String{
				"Alert activation scheduled to ",
				TimeStamp(activation.next).to_string().c_str()
			}.info(name());
		} else {
			activation.next = next;
			Logger::String{
				"Alert activation re-scheduled to ",
				TimeStamp(activation.next).to_string().c_str()
			}.info(name());
		}
		Controller::getInstance().wakeup();
	}

	bool Alert::deactivate() noexcept {

		if(!active()) {
			return false;
		}

		if(activation.running) {
			Logger::String{"Deactivating alert while running"}.warning(name());
		} else {
			Logger::String{"Deactivating alert"}.info(name());
		}

		reset(0);
		Controller::getInstance().wakeup();

		return true;
	}

	void Alert::success() noexcept {

		activation.suceeded++;

		if(activation.suceeded < retry.min) {

			// Retry.
			activation.next = time(0) + timers.interval;
			Logger::String{
				"Alert will retry at ",
				TimeStamp(activation.next).to_string().c_str(),
				" (",activation.suceeded,"/",retry.min,")"
			}.info(name());

		} else if(restart.success) {

			// Reactivate.
			reset(time(0) + restart.success);
			Logger::String{
				"Alert will be reactivated at ",
				TimeStamp(activation.next).to_string().c_str()
			}.info(name());

		} else {

			// Deactivate
			Logger::String{
				"Alert deactivated after ",
				activation.suceeded," successful activations"
			}.info(name());

			reset(0);

		}

		Controller::getInstance().wakeup();

	}

	void Alert::failed(const char *message) noexcept {

		activation.failed++;
		Logger::String{message," (",activation.failed,"/",retry.max,")"}.error(name());

		if(activation.failed < retry.max) {

			// Retry.
			activation.next = time(0) + timers.interval;
			Logger::String{
				"Alert will retry at ",
				TimeStamp(activation.next).to_string().c_str(),
				" (",activation.failed,"/",retry.max,")"
			}.info(name());

		} else if(restart.failed) {

			// Reactivate.
			reset(time(0) + restart.failed);
			Logger::String{
				"Alert will be reactivated at ",
				TimeStamp(activation.next).to_string().c_str()
			}.info(name());
	
		} else {

			// Deactivate
			Logger::String{
				"Alert deactivated after ",
				activation.failed," failed activations"
			}.info(name());
			reset(0);

		}

		Controller::getInstance().wakeup();

	}

 }

