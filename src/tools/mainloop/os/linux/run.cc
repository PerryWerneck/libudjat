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

 using namespace std;

 static void onInterruptSignal(int signal) noexcept {

 	// Use thread to avoid semaphore dead lock.
	Udjat::Application::warning() << "Received '" << strsignal(signal) << "' signal" << endl;
	Udjat::MainLoop::getInstance().quit();

 }

 void Udjat::MainLoop::run() {

	//
	// Start services
	//
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
 	while(this->enabled) {

		// Load lists.
		nfds_t nfds = 0;

		// Add event handle
		{
			fds[nfds].fd = efd;
			fds[nfds].events = POLLIN;
			nfds++;
		}

		// Get wait time, update timers.
		unsigned long wait = timers.run();
		nfds += getHandlers(&fds, &szPoll);

		// Wait for event.
		int nSocks = poll(fds, nfds, wait);

#ifdef DEBUG
		cout << "MainLoop\tnSocks=" << nSocks << " wait=" << wait << " nfds=" << nfds << endl;
#endif // DEBUG

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

		for(nfds_t sock = 0; sock < nfds && nSocks > 0; sock++) {

			int event = fds[sock].revents;

			if(!event)
				continue;

			nSocks--;

			if(fds[sock].fd == efd) {

				uint64_t evNum;
				if(read(efd, &evNum, sizeof(evNum)) != sizeof(evNum)) {
					cerr << "MainLoop\tError '" << strerror(errno) << "' reading event fd" << endl;
				}

#ifdef DEBUG
				cout << "MainLoop\tEvent FD was triggered" << endl;
#endif // DEBUG
				continue;
			}

			for(auto handle : handlers) {

				if(handle->fd == fds[sock].fd && (handle->events & fds[sock].events) != 0) {

					try {

						if(handle->fd > 0 && !handle->call((const Event) event))
							handle->fd = -1;

					} catch(const exception &e) {

						cerr << "MainLoop\tError '" << e.what() << "' processing event" << endl;

					} catch(...) {

						cerr << "MainLoop\tUnexpected error processing event" << endl;

					}

					break;

				}

			}

		}

 	}

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

 }

