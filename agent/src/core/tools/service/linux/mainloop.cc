/*
 *
 * Copyright (C) <2019> <Perry Werneck>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 *
 * @file
 *
 * @brief Implement linux main loop.
 *
 * @author Perry Werneck <perry.werneck@gmail.com>
 *
 *
 */

 #include "../private.h"
 #include <udjat/string.h>
 #include <csignal>
 #include <cstring>
 #include <sys/eventfd.h>
 #include <unistd.h>

 #if defined(HAVE_SYSTEMD)
	#include <syslog.h>
	#include <systemd/sd-daemon.h>
	#include <systemd/sd-journal.h>
 #endif // HAVE_SYSTEMD

 namespace Udjat::Service {

	void Controller::terminate(int signal) noexcept {

		warning("Interrupted by signal \"",strsignal(signal),"\"");
		getInstance().enabled = false;

	}

	void Controller::reload(int signal) noexcept {

		warning("Reloading by signal \"",strsignal(signal),"\"");

		lock_guard<mutex> lock(Abstract::EventListener::mtx);
		for(auto listener : getInstance().listeners) {
			info("Reloading ",listener->name);
			listener->reload();
		}

	}

	void Controller::run() {

		struct {
			int				id;			///< @brief Signal id.
			sighandler_t	mine;		///< @brief My signal handler.
			sighandler_t	original;	///< @brief Saved signal handler.
		} siglist[] = {

			{ SIGTERM,	Controller::terminate,	NULL	},		// termination request, sent to the program
			{ SIGINT,	Controller::terminate,	NULL	},		// external interrupt, usually initiated by the user
			{ SIGHUP,	Controller::reload,		NULL	},		// Reload
//			{ SIGABRT,	Controller::terminate,	NULL	}		// abnormal termination condition, as is e.g. initiated by std::abort()

		};

		for(size_t it = 0; it != N_ELEMENTS(siglist); it++) {
			siglist[it].original = signal(siglist[it].id,siglist[it].mine);
		}

#ifdef HAVE_SYSTEMD
		sd_notifyf(0,"READY=1\nSTATUS=Service loop is running\nMAINPID=%lu",(unsigned long) getpid());
		sd_journal_print(LOG_NOTICE, "Service loop is active");
#endif // HAVE_SYSTEMD

		fdevent = eventfd(0,EFD_NONBLOCK);
		if(fdevent < 0) {
			throw system_error(errno, system_category());
		}

		enabled = true;

		// Notify event listeners
		{
			lock_guard<mutex> lock(Abstract::EventListener::mtx);
			for(auto listener : listeners) {
				info("Starting ",listener->name);
				if(!listener->isActive()) {
					listener->start();
				}
			}
		}

		while(enabled) {

			pollfd	* fds = nullptr;
			nfds_t	  nfds;
			time_t	  wait = 60;

			// Manage timers
			{
				list<Abstract::Timer *> expiredTimers;
				time_t now = time(nullptr);

				{
					lock_guard<mutex> lock(mtx);

					for(auto timer : timers) {
						if(timer->next > now) {

							wait = std::min(wait,(timer->next - now));

						} else if(timer->interval) {

							expiredTimers.push_back(timer);
							timer->next = now+timer->interval;
							wait = std::min(wait,(timer->next - now));

						}
					}

				}

				// Run timers;
				for(auto timer : expiredTimers) {

					try {

						timer->onTimer();

					} catch(const std::exception &e) {

						error("Error processing timer: ",e.what());

					} catch(...) {

						error("Unexpected error processing timer");

					}

				}

			}

			// Cleanup handlers (Should be done after the timers).
			{
				lock_guard<mutex> lock(mtx);
				handlers.remove_if([](Handler &handler){
					return handler.fd < 0;
				});

			}

			// Create fd list.
			{
				lock_guard<mutex> lock(mtx);

				size_t sz = handlers.size()+1;

				fds = new pollfd[sz];
				memset(fds,0,sz * sizeof(pollfd));
				nfds = 0;

				for(auto handler : handlers) {

					if(handler.fd > 0) {
						fds[nfds].fd = handler.fd;
						fds[nfds].events = handler.ev;
						nfds++;
					}

				}

				// Last entry is for event socket.
				fds[nfds].fd = fdevent;
				fds[nfds].events = POLLIN|POLLERR|POLLHUP;

				nfds++;

			}

			// Wait for event.
			debug("Wait=",wait);
			int nSocks = poll(fds, nfds, wait * 1000);
			debug("nSocks=",nSocks);

			if(nSocks < 0) {

				// Poll error
				if(errno != EINTR) {

					int state = errno;

#ifdef HAVE_SYSTEMD
					sd_notifyf(0,"STATUS=Polling error: %s\nERRNO=%d",strerror(state),state);
#endif // HAVE_SYSTEMD

					error("Mainloop has failed: ",strerror(state));
					enabled = false;

				}

			} else if(nSocks) {

				debug("fds[",0,"].fd=",fds[0].fd," fdevent=",fdevent);

				// Process events.
				for(nfds_t nfd = 0; nfd < nfds && nSocks > 0; nfd++) {

					if(fds[nfd].revents) {

						nSocks--;

						std::function<bool(int fd, enum Event ev)> callback;

						if(fds[nfd].fd != fdevent) {

							// Is not the fdevent, scan for handler.
							lock_guard<mutex> lock(mtx);

							for(auto handler : handlers) {

								if(fds[nfd].fd == handler.fd && fds[nfd].events == handler.ev) {
									callback = handler.callback;
									break;
								}

							}

						} else {

							uint64_t evNum;
							if(read(fdevent, &evNum, sizeof(evNum)) != sizeof(evNum)) {
								warning("Can't read from event socket: ",strerror(errno));
							}

						}

						if(callback) {

							try {

								callback(fds[nfd].fd, (enum Event) fds[nfd].revents);

							} catch(const std::exception &e) {

								error("Error processing event on fd ", fds[nfd].fd, ": ", e.what());

							} catch(...) {

								error("Unexpected error processing event on fd ",fds[nfd].fd);
							}

						}

					}

				}

			}

			// Cleanup
			delete[] fds;

		}
		{
			lock_guard<mutex> lock(Abstract::EventListener::mtx);
			for(auto listener : listeners) {
				info("Stopping ",listener->name);
				if(listener->isActive()) {
					listener->stop();
				}
			}
		}

		close(fdevent);
		fdevent = -1;

#ifdef HAVE_SYSTEMD
		sd_notify(0,"STATUS=Service loop was ended");
		sd_journal_print(LOG_NOTICE, "Service loop is inactive");
#endif // HAVE_SYSTEMD

		for(size_t it = 0; it != N_ELEMENTS(siglist); it++) {
			signal(siglist[it].id,siglist[it].original);
		}

	}

	/// @brief Wake up main loop
	void Controller::wakeup() {
		if(fdevent > 0) {
			static uint64_t evNum = 1;
			if(write(fdevent, &evNum, sizeof(evNum)) != sizeof(evNum)) {
				warning("Can't write event handler: ", strerror(errno));
			}
			evNum++;
		}
	}

 }

