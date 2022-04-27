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
 #include <cstring>
 #include <udjat-internals.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/application.h>
 #include <iostream>
 #include <unistd.h>
 #include <csignal>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 static void onInterruptSignal(int signal) noexcept {

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"STATUS=Interrupting by '%s' signal",strsignal(signal));
#endif // HAVE_SYSTEMD

	Udjat::Application::warning() << "Received '" << strsignal(signal) << "' signal" << endl;
	Udjat::MainLoop::getInstance().quit();

 }

 void Udjat::MainLoop::run() {

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
	signal(SIGTERM,onInterruptSignal);
	signal(SIGINT,onInterruptSignal);

 	//
 	// Main event loop
 	//
	nfds_t szPoll = 2;
	struct pollfd *fds = (pollfd *) malloc(sizeof(struct pollfd) *szPoll);
	memset(fds,0,sizeof(struct pollfd) *szPoll);

 	this->enabled = true;
#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"READY=1\nSTATUS=Running");
#endif // HAVE_SYSTEMD
 	while(this->enabled) {

		// Get wait time, update timers.
		unsigned long wait = timers.run();
		nfds_t nfds = getHandlers(&fds, &szPoll);

 		// EventFD in the last entry.
		{
			fds[nfds].fd = efd;
			fds[nfds].events = POLLIN;
		}

		// Wait for event.
		int nSocks = poll(fds, nfds+1, wait);

//#ifdef DEBUG
//		cout << "MainLoop\tnSocks=" << nSocks << " wait=" << wait << " nfds=" << nfds << endl;
//		for(size_t ix = 0; ix < nfds;ix++) {
//			cout << ix << " = " << fds[ix].revents << "  (" << &fds[ix] << ")" << endl;
//		}
//#endif // DEBUG

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
		if(fds[nfds].revents) {
			uint64_t evNum;
//#ifdef DEBUG
//			cout << "MainLoop\tEventFD was changed" << endl;
//#endif // DEBUG
			if(read(efd, &evNum, sizeof(evNum)) != sizeof(evNum)) {
				cerr << "MainLoop\tError '" << strerror(errno) << "' reading event fd" << endl;
			}
			nSocks--;
		}

		// Check for fd handlers.
		{
			// First, get list of the active handlers.
			std::list<std::shared_ptr<Handler>> hList;

			{
				lock_guard<mutex> lock(guard);
				for(auto handle : handlers) {
					if(handle->index >= 0 && fds[handle->index].revents) {
//						cout << "*** " << handle->id() << " events=" << fds[handle->index].revents << endl;
						hList.push_back(handle);
					}
				}
			}

			// Second, call handlers
			for(auto handle : hList) {

				try {

					if(!handle->call((const Event) fds[handle->index].revents)) {
						lock_guard<mutex> lock(guard);
						handlers.remove(handle);
					}

				} catch(const std::exception &e) {

					cerr << "MainLoop\tError '" << e.what() << "' processing FD(" << handle->fd << "), disabling it" << endl;
					handle->disable();

				} catch(...) {

					cerr << "MainLoop\tUnexpected error processing FD(" << handle->fd << "), disabling it" << endl;
					handle->disable();

				}

			}

		}

 	}
#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"STOPPING=1\nSTATUS=Stopping");
#endif // HAVE_SYSTEMD

 	free(fds);

 	//
 	// Restore signals
 	//
	signal(SIGTERM,SIG_DFL);
	signal(SIGINT,SIG_DFL);

	//
 	// Stop services
 	//
	stop();

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"STATUS=Stopped");
#endif // HAVE_SYSTEMD
 }

