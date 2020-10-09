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
 #include <cstring>

 #ifdef HAVE_SIGNAL
	#include <csignal>
 #endif // HAVE_SIGNAL

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
		cerr << "MainLoop\tError '" << strerror(errno) << "' writing to event loop" << endl;
	}
#endif // HAVE_EVENTFD
 }

#ifdef HAVE_SIGNAL
 void Udjat::Service::Controller::onInterruptSignal(int signal) noexcept {

 	clog << "MainLoop\tStopping by '" << strsignal(signal) << "' signal" << endl;

#ifdef HAVE_SYSTEMD
	{
		string msg = "STOPPING=1\nSTATUS=Stopping by '";
		msg += strsignal(signal);
		msg += "' signal";
		sd_notify(0,msg.c_str());
	}
#endif // HAVE_SYSTEMD

 	Controller &controller = getInstance();
 	controller.enabled = false;
 	controller.wakeup();

 }
#endif // HAVE_SIGNAL

 void Udjat::Service::Controller::run() noexcept {

	/// Poll.
	nfds_t szPoll = 2;
	struct pollfd *fds = (pollfd *) malloc(sizeof(struct pollfd) *szPoll);

	/// @brief Threads to run callback methods.
 	ThreadPool threads{"service-events"};
 	threads.setMaxThreads(2);

#ifdef HAVE_SIGNAL
 	// Intercept signals
 	sighandler_t sigterm = signal(SIGTERM,onInterruptSignal);
 	sighandler_t sigint = signal(SIGINT,onInterruptSignal);
#endif // HAVE_SIGNAL

 	//
 	// Main event loop
 	//
#ifdef HAVE_SYSTEMD
	sd_notifyf(0,"READY=1\nMAINPID=%lu",(unsigned long) getpid());
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

				if(timer.next <= now) {

					// Still have pending events? Ignore it but keep me the list.
					if(timer.running) {
						clog << "MainLoop\tTimer call is taking too long" << endl;
						return false;
					}

					// Timer has expired, update value and enqueue method.
					timer.next = (now + timer.seconds);

					timer.running = now;
					threads.push(timer.name,[&timer,now]() {

						try {

							if(timer.seconds && !timer.call(now))
								timer.seconds = 0;

						} catch(const exception &e) {

							cerr << "MainLoop\tTimer error '" << e.what() << "'" << endl;

						} catch(...) {

							cerr << "MainLoop\tUnexpected error on timer" << endl;

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

			cerr << "MainLoop\tError '" << e.what() << "' processing events" << endl;
			this->enabled = false;
			continue;

		} catch(...) {

			cerr << "MainLoop\tUnexpected error processing events" << endl;
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
					cerr << "MainLoop\tError '" << strerror(errno) << "' reading event fd" << endl;
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

							cerr << "MainLoop\tError '" << e.what() << "' processing file/socket event" << endl;

						} catch(...) {

							cerr << "MainLoop\tUnexpected error processing file/socket event" << endl;

						}

						handle.running = 0;

					});

					break;
				}

			}

		}

 	}

 	free(fds);

#ifdef HAVE_SIGNAL
 	// Restore signals
	signal(SIGTERM,sigterm);
	signal(SIGINT,sigint);
#endif // HAVE_SIGNAL

#ifdef HAVE_SYSTEMD
	sd_notify(0,"STATUS=Service loop was ended");
#endif // HAVE_SYSTEMD

 }

