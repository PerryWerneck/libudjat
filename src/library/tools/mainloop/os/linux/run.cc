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

/**
 * @file
 *
 * @brief Implement linux main loop.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/misc.h>
 #include <private/linux/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/timer.h>
 #include <iostream>
 #include <unistd.h>
 #include <udjat/tools/event.h>

 #include <csignal>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 static const int signals[] = { SIGTERM, SIGINT };

 int Udjat::Linux::MainLoop::run() {

	//
	// Capture signals
	//
	for(size_t signal = 0; signal < (sizeof(signals)/sizeof(signals[0]));signal++) {

			Logger::String{
				"Signal '",(const char *) strsignal(signals[signal]),"' (",signals[signal],") will trigger a controlled stop"
			}.write(Logger::Trace,"signal");

			Udjat::Event::SignalHandler(this,signals[signal],[this](){

				std::thread{[this](){

					debug("Stopping main loop by signal");
#ifdef HAVE_SYSTEMD
					sd_notify(0,"STATUS=Interrupting by signal");
#endif // HAVE_SYSTEMD

					quit();

				}}.detach();

				return true;
			});

	}

 	//
 	// Main event loop
 	//

 	this->running = true;

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"READY=1");
#endif // HAVE_SYSTEMD

 	while(this->running) {

		// Get wait time, update timers.
		unsigned long wait = compute_poll_timeout();

		// Get handlers
		size_t maxfd = handlers.size()+2;
		struct pollfd fds[maxfd];
		Handler *hList[maxfd];

		// Clear
		nfds_t nfds = 0;

		memset(fds,0,maxfd * sizeof(pollfd));
		memset(hList,0,maxfd * sizeof(Handler *));

 		// EventFD in the first entry.
		{
			fds[nfds].fd = efd;
			fds[nfds].events = POLLIN;
			nfds++;
		}

		{
			lock_guard<mutex> lock(guard);
			for(auto handle : handlers) {
				hList[nfds] = handle;
				handle->get(fds[nfds]);
				nfds++;
			}
		}

		// Wait for event.
		debug("------------------------->",nfds);
		evNum++;
		int nSocks = poll(fds, nfds, wait);
		if(nSocks == 0) {
			continue;
		}

		if(nSocks < 0) {

			if(!this->running) {
				break;
			}

			if(errno != EINTR) {
				cerr << "MainLoop\tError '" << strerror(errno) << "' (" << errno << ") running mainloop, stopping" << endl;
				this->running = false;
			}

			continue;

		}

		// Check for event fd.
		if(fds[0].revents) {
			uint64_t evNum;
			if(read(efd, &evNum, sizeof(evNum)) != sizeof(evNum)) {
				cerr << "MainLoop\tError '" << strerror(errno) << "' reading event fd" << endl;
			}
			nSocks--;
		}

		while(nSocks > 0) {

			for(size_t ix=0; ix < nfds; ix++) {

				if(fds[ix].revents) {

					if(enabled(hList[ix])) {

						try {

							hList[ix]->set(fds[ix]);

						} catch(const std::exception &e) {

							cerr << "MainLoop\tError '" << e.what() << "' processing handler, disabling it" << endl;
							hList[ix]->disable();

						} catch(...) {

							cerr << "MainLoop\tUnexpected error processing handler, disabling it" << endl;
							hList[ix]->disable();

						}

					}
					nSocks--;
				}

			}

		}

 	}

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"STOPPING=1");
#endif // HAVE_SYSTEMD

 	//
 	// Restore signals
 	//
	Udjat::Event::remove(this);

	return 0;

 }

 unsigned long Udjat::Linux::MainLoop::compute_poll_timeout() noexcept {

	unsigned long now = MainLoop::Timer::getCurrentTime();
	unsigned long next = now + timers.maxwait;

	// Get expired timers.
	std::list<Timer *> expired;
	for_each([&expired,&next,now](Timer &timer){
		if(timer.activation_time() <= now) {
			debug("activation=",timer.activation_time());
			expired.push_back(&timer);
		} else {
			next = std::min(next,timer.activation_time());
			debug("next=",now-next);
		}
		return false;
	});

	debug("expired=",expired.size());

	// Run expired timers.
	for(auto timer : expired) {
		unsigned long n = timer->activate();
		debug("after activation=",now-n);
		if(n) {
			next = std::min(next,n);
		}
	}

	if(next > now) {
		debug("Time interval ",(next-now)," ms (",TimeStamp{time(0) + ((time_t) ((next-now)/1000))}.to_string(),")");
		return (next - now);
	}

	Logger::String{"Unexpected interval '",((int) (next - now)),"' on timer processing, using default"}.error();

	return timers.maxwait;
 }

