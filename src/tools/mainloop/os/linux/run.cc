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
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/logger.h>
 #include <iostream>
 #include <unistd.h>
 #include <udjat/tools/event.h>

// #include <cstring>
 #include <csignal>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 static const int signals[] = { SIGTERM, SIGINT };

 int Udjat::MainLoop::run() {

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"MAINPID=%lu",(unsigned long) getpid());
#endif // HAVE_SYSTEMD

	//
	// Start services
	//
#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"STATUS=Starting up");
#endif // HAVE_SYSTEMD
	start();

	//
	// Capture signals
	//
	for(size_t signal = 0; signal < (sizeof(signals)/sizeof(signals[0]));signal++) {

			Logger::String{
				"Signal '",strsignal(signals[signal]),"' (",signals[signal],") will trigger a controlled stop"
			}.write(Logger::Trace,"signal");

			Udjat::Event::SignalHandler(this,signals[signal],[this](){

#ifdef HAVE_SYSTEMD
				sd_notify(0,"STATUS=Interrupting by signal");
#endif // HAVE_SYSTEMD

				Udjat::Application::warning() << "Interrupting main loop" << endl;
				Udjat::MainLoop::getInstance().quit();

				return true;
			});

	}
	/*
	signal(SIGTERM,onInterruptSignal);
	signal(SIGINT,onInterruptSignal);
	*/

 	//
 	// Main event loop
 	//

 	this->enabled = true;

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"READY=1\nSTATUS=Running");
#endif // HAVE_SYSTEMD

 	while(this->enabled) {

		// Get wait time, update timers.
		unsigned long wait = timers.run();

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
				fds[nfds].fd = handle->fd;
				fds[nfds].events = handle->events;
				fds[nfds].revents = 0;
				nfds++;
			}
		}

		// Wait for event.
		int nSocks = poll(fds, nfds, wait);

		if(nSocks == 0) {
			continue;
		}

		if(nSocks < 0) {

			if(!this->enabled) {
				break;
			}

			if(errno != EINTR) {
				cerr << "MainLoop\tError '" << strerror(errno) << "' (" << errno << ") running mainloop, stopping" << endl;
				this->enabled = false;
			}

			continue;

		}

		// Check for event fd.
		if(fds[0].revents) {
			// debug("Wake UP received");
			uint64_t evNum;
			if(read(efd, &evNum, sizeof(evNum)) != sizeof(evNum)) {
				cerr << "MainLoop\tError '" << strerror(errno) << "' reading event fd" << endl;
			}
			nSocks--;
		}

		while(nSocks > 0) {

			for(size_t ix=0; ix < nfds; ix++) {

				if(fds[ix].revents) {

					if(verify(hList[ix])) {

						try {

							hList[ix]->handle_event((const Handler::Event) fds[ix].revents);

						} catch(const std::exception &e) {

							cerr << "MainLoop\tError '" << e.what() << "' processing FD(" << hList[ix]->fd << "), disabling it" << endl;
							hList[ix]->disable();

						} catch(...) {

							cerr << "MainLoop\tUnexpected error processing FD(" << hList[ix]->fd << "), disabling it" << endl;
							hList[ix]->disable();

						}

					}
					nSocks--;
				}

			}

		}

 	}

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"STOPPING=1\nSTATUS=Stopping");
#endif // HAVE_SYSTEMD

 	//
 	// Restore signals
 	//
	Udjat::Event::remove(this);

	//
 	// Stop services
 	//
	stop();

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"STATUS=Stopped");
#endif // HAVE_SYSTEMD

	return 0;

 }

