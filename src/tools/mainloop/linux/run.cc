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
 #include "../private.h"
 #include <udjat/tools/threadpool.h>

 void Udjat::MainLoop::run() {

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
		time_t wait = timers.run();

		nfds += getHandlers(&fds, &szPoll);

		// Wait for event.
		int nSocks = poll(fds, nfds, wait * 1000);

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

