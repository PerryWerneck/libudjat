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
 #include <cstring>
 #include <sys/eventfd.h>
 #include <private/misc.h>
 #include <udjat/tools/mainloop.h>
 #include <private/linux/mainloop.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/timer.h>
 #include <iostream>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

 	std::mutex Linux::MainLoop::guard;

	Linux::MainLoop::MainLoop() : Udjat::MainLoop{MainLoop::Pool} {
		efd = eventfd(0,0);
		if(efd < 0)
			throw system_error(errno,system_category(),"eventfd() has failed");
	}

	Linux::MainLoop::~MainLoop() {

		if(!handlers.empty()) {
			cerr << "MainLoop\tDestroying mainloop with " << handlers.size() << " pending handler(s)" << endl;
		} else {
			Logger::String{"Destroying clean service loop"}.trace("MainLoop");
		}

		running = false;
		wakeup();

		{
			lock_guard<mutex> lock(guard);
			::close(efd);
			efd = -1;
		}

		debug("Mainloop was destroyed");

	}

	void Linux::MainLoop::wakeup() noexcept {
		if(efd != -1) {
			static uint64_t evNum = 0;
			if(write(efd, &evNum, sizeof(evNum)) != sizeof(evNum)) {
				cerr << "MainLoop\tError '" << strerror(errno) << "' writing to event loop using fd " << efd << endl;
			}
			evNum++;
		} else {
			clog << "MainLoop\tUnexpected call with efd=" << efd << endl;
		}
	}

	bool Linux::MainLoop::enabled(const Timer *timer) const noexcept {
		lock_guard<mutex> lock(guard);
		for(Timer *tm : timers.enabled) {
			if(timer == tm) {
				return true;
			}
		}
		return false;
	}

	bool Linux::MainLoop::enabled(const MainLoop::Handler *handler) const noexcept {
		lock_guard<mutex> lock(guard);
		for(auto hdl : handlers) {
			if(handler == hdl) {
				return true;
			}
		}
		return false;
	}

	bool Linux::MainLoop::active() const noexcept {
		return this->running;
	}

	void Linux::MainLoop::post(Message *message) noexcept {

		try {
			message->execute();
		} catch(const std::exception &e) {
			Logger::String{"Error processing posted message: ",e.what()}.error();
		} catch(...) {
			Logger::String{"Unexpected rror processing posted message"}.error();
		}
		delete message;

	}

	void Linux::MainLoop::push_back(MainLoop::Timer *timer) {
		lock_guard<mutex> lock(guard);
#ifdef DEBUG
		cout << "MainLoop\t---> Enabling timer " << hex << ((void *) timer) << dec
				<< " " << timer->to_string() << endl;
#endif // DEBUG
		timers.enabled.push_back(timer);
		wakeup();
	}

	void Linux::MainLoop::remove(MainLoop::Timer *timer) {
#ifdef DEBUG
		clog << "MainLoop\t---> Disabling timer " << hex << ((void *) timer) << dec << endl;
#endif // DEBUG
		lock_guard<mutex> lock(guard);
#ifdef DEBUG
		clog << "MainLoop\t---> Disabling timer " << hex << ((void *) timer) << dec << endl;
#endif // DEBUG
		timers.enabled.remove(timer);
	}

	void Linux::MainLoop::push_back(MainLoop::Handler *handler) {
		lock_guard<mutex> lock(guard);
		handlers.push_back(handler);
		wakeup();
	}

	void Linux::MainLoop::remove(MainLoop::Handler *handler) {
		lock_guard<mutex> lock(guard);
		handlers.remove(handler);
		wakeup();
	}

	bool Linux::MainLoop::for_each(const std::function<bool(Timer &timer)> &func) {
		lock_guard<mutex> lock(guard);
		for(auto timer : timers.enabled) {
			if(func(*timer)) {
				return true;
			}
		}
		return false;
	}

 }
