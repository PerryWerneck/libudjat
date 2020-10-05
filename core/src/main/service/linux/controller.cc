/**
 * @file
 *
 * @brief Implement linux main loop.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "../private.h"
 #include <cstring>
 #include <udjat/tools/threadpool.h>
 #include <poll.h>
 #include <malloc.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #if defined(HAVE_SYSTEMD)
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 void Udjat::Service::Controller::wakeup() noexcept {
#ifdef HAVE_EVENTFD
	static uint64_t evNum = 1;
	if(write(efd, &evNum, sizeof(evNum)) != sizeof(evNum)) {
		cerr << "Can't write to event handler: " << strerror(errno) << endl;
	}
#endif // HAVE_EVENTFD
 }

 void Udjat::Service::Controller::run() noexcept {

	/// Poll.
	nfds_t szPoll = 2;
	struct pollfd *fds = (pollfd *) malloc(sizeof(struct pollfd) *szPoll);

	/// @brief Threads to run callback methods.
 	ThreadPool threads;
 	threads.setMaxThreads(2);

#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"READY=1\nSTATUS=Service loop is running\nMAINPID=%lu",(unsigned long) getpid());
#endif // HAVE_SYSTEMD

 	this->enabled = true;
 	while(this->enabled) {

#ifdef HAVE_EVENTFD
			time_t wait = 60;
#else
			time_t wait = 1;
#endif // HAVE_EVENTFD

			// Load lists.
			nfds_t nfds = 0;
#ifdef HAVE_EVENTFD
			{
				fds[nfds].fd = efd;
				fds[nfds].events = POLLIN;
				nfds++;
			}
#endif // HAVE_EVENTFD

			{
				time_t now = time(nullptr);
				lock_guard<recursive_mutex> lock(guard);

				// Get wait time, update timers.
				timers.remove_if([now,&wait,&threads](Timer &timer) {

					if(!timer.seconds)
						return threads.empty();

					if(timer.next <= now) {

						// Timer has expired, update value and enqueue method.
						timer.next = (now + timer.seconds);

						threads.push([&timer,now]() {

							try {

								if(timer.seconds && !timer.call(now))
									timer.seconds = 0;

							} catch(const exception &e) {

								cerr << "Timer error: " << e.what() << endl;

							} catch(...) {

								cerr << "Unexpected error on timer" << endl;

							}
						});

					}

					wait = std::min(wait, (timer.next - now));

					return false;

				});

				// Get waiting sockets.
				handlers.remove_if([&szPoll,&fds,&nfds,&threads](Handle &handle) {

					if(handle.fd <= 0)
						return threads.empty();

					if(nfds >= szPoll) {
						szPoll += 2;
						fds = (struct pollfd *) realloc(fds, sizeof(struct pollfd) * szPoll);
					}

					fds[nfds].fd = handle.fd;
					fds[nfds].events = handle.events;
					nfds++;
					return false;
				});

			}

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
						cerr << "Error reading eventfd: " << strerror(errno) << endl;
					}

					continue;
				}
#endif // HAVE_EVENTFD

				for(auto handle : handlers) {

					if(handle.fd == fds[sock].fd && (handle.events & fds[sock].events) != 0) {

						threads.push([&handle,event]() {

							try {

								if(handle.fd > 0 && !handle.call((const Event) event))
									handle.fd = -1;

							} catch(const exception &e) {

								cerr << "File/socket event error: " << e.what() << endl;

							} catch(...) {

								cerr << "Unexpected file/socket event error" << endl;

							}

						});

						break;
					}

				}

			}

 	}

 	free(fds);

#ifdef HAVE_SYSTEMD
	sd_notify(0,"STATUS=Service loop was ended");
#endif // HAVE_SYSTEMD

 }

