/**
 * @file
 *
 * @brief Implement linux main loop.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "../private.h"

 void Udjat::MainLoop::run() {

	/// Poll.
	nfds_t szPoll = 2;
	struct pollfd *fds = (pollfd *) malloc(sizeof(struct pollfd) *szPoll);

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
		time_t wait = runTimers(this->wait);
		nfds += getHandlers(&fds, &szPoll);

		// Wait for event.
		int nSocks = poll(fds, nfds, wait * 1000);

		for(nfds_t sock = 0; sock < nfds && nSocks > 0; sock++) {

			int event = fds[sock].revents;

			if(!event)
				continue;

			nSocks--;

#ifdef HAVE_EVENTFD
			if(fds[sock].fd == efd) {

				uint64_t evNum;
				if(read(efd, &evNum, sizeof(evNum)) != sizeof(evNum)) {
					cerr << "MainLoop\tError '" << strerror(errno) << "' reading event fd" << endl;
				}

				continue;
			}
#endif // HAVE_EVENTFD

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

 }

