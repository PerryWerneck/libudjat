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
 #include <iostream>
 #include <unistd.h>

 using namespace std;

 void Udjat::MainLoop::run(bool service) {

	/// Poll.
	nfds_t szPoll = 2;
	struct pollfd *fds = (pollfd *) malloc(sizeof(struct pollfd) *szPoll);
	memset(fds,0,sizeof(struct pollfd) *szPoll);

	//
	// Start services
	//
	{
		lock_guard<mutex> lock(Service::guard);
		cout << "mainloop\tStarting " << services.size() << " service(s)" << endl;
		for(auto service : services) {
			if(!service->active) {
				try {
					cout << "service\tStarting '" << service->info->description << " " << service->info->version << "'" << endl;
					service->start();
					service->active = true;
				} catch(const std::exception &e) {
					cerr << service->info->name << "\tError '" << e.what() << "' starting service" << endl;
				}
			}
		}
	}

 	//
 	// Main event loop
 	//
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

				continue;
			}

			for(auto handle = handlers.begin(); handle != handlers.end(); handle++) {

				if(handle->fd == fds[sock].fd && (handle->events & fds[sock].events) != 0 && !handle->running) {

					handle->running = time(nullptr);

					try {

						if(handle->fd > 0 && !handle->call((const Event) event))
							handle->fd = -1;

					} catch(const exception &e) {

						cerr << "MainLoop\tError '" << e.what() << "' processing event" << endl;

					} catch(...) {

						cerr << "MainLoop\tUnexpected error processing event" << endl;

					}

					handle->running = 0;
					break;

				}

			}

		}

 	}

 	free(fds);

 	//
	// Stop services
	//
	{
		lock_guard<mutex> lock(Service::guard);
		cout << "mainloop\tStopping " << services.size() << " service(s)" << endl;
		for(auto service : services) {
			if(service->active) {
				try {
					cout << "service\tStopping '" << service->info->description << " " << service->info->version << "'" << endl;
					service->stop();
				} catch(const std::exception &e) {
					cerr << service->info->name << "\tError '" << e.what() << "' stopping service" << endl;
				}
				service->active = false;
			}
		}
	}

	// Wait for pool
	ThreadPool::getInstance().wait();

 }

