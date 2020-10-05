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

			try {

				time_t now = time(nullptr);
				lock_guard<recursive_mutex> lock(guard);

				// Get wait time, update timers.
				timers.remove_if([now,&wait,&threads](Timer &timer) {

					// Do I still active? If not return true *only* if not running.
					if(!timer.seconds)
						return (timer.running != 0);

					// Still have pending events? Ignore it but keep me the list.
					if(timer.running)
						return false;

					if(timer.next <= now) {

						// Timer has expired, update value and enqueue method.
						timer.next = (now + timer.seconds);

						timer.running = now;
						threads.push([&timer,now]() {

							try {

								if(timer.seconds && !timer.call(now))
									timer.seconds = 0;

							} catch(const exception &e) {

								cerr << "Timer error: " << e.what() << endl;

							} catch(...) {

								cerr << "Unexpected error on timer" << endl;

							}

							timer.running = 0;

						});

					}

					wait = std::min(wait, (timer.next - now));

					return false;

				});

				// Get waiting sockets.
				handlers.remove_if([&szPoll,&fds,&nfds,&threads](Handle &handle) {

					// Are we active? If not return true *only* if theres no pending event.
					if(handle.fd <= 0)
						return handle.running == 0;

					// Am I running? If yes don't pool for me, but keep me in the list.
					if(handle.running)
						return false;

					if(nfds >= szPoll) {
						szPoll += 2;
						fds = (struct pollfd *) realloc(fds, sizeof(struct pollfd) * szPoll);
					}

					fds[nfds].fd = handle.fd;
					fds[nfds].events = handle.events;
					nfds++;
					return false;
				});

			} catch(const std::exception &e) {

				cerr << "Mainloop processing has failed: " << e.what() << endl;
				this->enabled = false;
				continue;

			} catch(...) {

				cerr << "Unexpected error on mainloop" << endl;
				this->enabled = false;
				continue;

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

						handle.running = time(nullptr);
						threads.push([&handle,event]() {

							try {

								if(handle.fd > 0 && !handle.call((const Event) event))
									handle.fd = -1;

							} catch(const exception &e) {

								cerr << "File/socket event error: " << e.what() << endl;

							} catch(...) {

								cerr << "Unexpected file/socket event error" << endl;

							}

							handle.running = 0;

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

