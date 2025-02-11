/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/socket.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/socket.h>
 
 #include <netdb.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <unistd.h>
 #include <string.h>
 #include <sys/ioctl.h>
 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	Socket::Socket(const URL &url, unsigned int seconds) {

		if(seconds < 1) {
			seconds = Config::Value<unsigned int>("network","timeout",5000).get();
		}

        struct addrinfo   hints;
        struct addrinfo * result        = NULL;
        memset(&hints,0,sizeof(hints));

        hints.ai_family         = AF_UNSPEC;    // Allow IPv4 or IPv6
        hints.ai_socktype       = SOCK_STREAM;  // Stream socket
        hints.ai_flags          = AI_PASSIVE;   // For wildcard IP address
        hints.ai_protocol       = 0;            // Any protocol

		// TODO: Use getaddrinfo_a to resolve in parallel
        int rc = getaddrinfo(url.hostname().c_str(), url.servicename().c_str(), &hints, &result);
        if(rc) {
			throw Exception(rc, Logger::String{"Failed to resolve '",url.c_str(),"'"},gai_strerror(rc));
		}

		int error = 0;
		int sock = -1;
		for(struct addrinfo *rp = result; rp; rp = rp->ai_next) {

			sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if(sock < 0) {
				continue;
			}

			int f;
            if ((f = fcntl(sock, F_GETFL, 0)) == -1) {
				error = errno;
				::close(sock);
				sock = -1;
				continue;
			}

  			f |= O_NDELAY;
            if (fcntl(sock, F_SETFL, f) < 0) {
				error = errno;
				::close(sock);
				sock = -1;
				continue;
			}

			if(::connect(sock,rp->ai_addr, rp->ai_addrlen) && errno != EINPROGRESS) {
				error = errno;
				::close(sock);
				sock = -1;
				continue;
			}

			break;
		}

		freeaddrinfo(result);

		if(sock < 0 && error > 0) {

			if(error > 0) {
				throw system_error(error,system_category(),Logger::String{"Failed to connect to '",url.c_str(),"'"});
			}
	
			throw runtime_error(Logger::String{"Failed to connect to '",url.c_str(),"'"});
		}

		connecting = true;
		set(sock);
		set(Handler::onoutput);
		enable();

	}

	Socket::Socket(int fd) : MainLoop::Handler(fd,(Event) (MainLoop::Handler::oninput|MainLoop::Handler::onhangup|MainLoop::Handler::onerror)) {
	}
		
	Socket::~Socket() {
		close();
	}

	void Socket::close() {
		close(values.fd);
		values.fd = -1;
	}

	void Socket::close(int sock) noexcept {
		if(sock >= 0) {
			if(::close(sock)) {
				Logger::String{"Error '",strerror(errno),"' closing socket ",sock}.warning();
			}
		}
	}

	void Socket::blocking(int sock, bool enable) {
		
		int flags = fcntl(sock, F_GETFL, 0);
		
		if(flags < 0) {
			throw system_error(errno,system_category(),"Failed to get socket flags");
		}

		if(enable) {
			flags &= ~O_NDELAY;
		} else {
			flags |= O_NDELAY;
		}

		if(fcntl(sock, F_SETFL, flags) < 0) {
			throw system_error(errno,system_category(),"Failed to set socket flags");
		}

	}

	int Socket::wait_for_connection(int sock, unsigned int seconds) {

		if(seconds < 1) {
			seconds = Config::Value<unsigned int>("network","timeout",10).get();
		}

		debug("Will wait for ",seconds," seconds to connect to ",sock);
		
		time_t timeout = time(0) + seconds;
		MainLoop &mainloop = MainLoop::getInstance();

		struct pollfd pfd;
		while(time(0) < timeout) {

			pfd.fd = sock;
			pfd.revents = 0;
			pfd.events = POLLOUT|POLLERR|POLLHUP;
#ifdef DEBUG
			auto rc = ::poll(&pfd,1,1000);
#else
			auto rc = ::poll(&pfd,1,100);
#endif // DEBUG
			if(rc == -1) {

				int error = errno;
				::close(sock);
				errno = error;
				return -1;
				
			} else if(rc == 1) {

				if(pfd.revents & POLLERR) {

					int error = EINVAL;
					socklen_t errlen = sizeof(error);
					if(getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen) < 0) {
						error = errno;
					}
					::close(sock);
					errno = error;
					return -1;
				}

				if(pfd.revents & POLLHUP) {
					::close(sock);
					errno = ECONNRESET;
					return -1;
				}

				if(pfd.revents & POLLOUT) {

					// Check socket error
					struct sockaddr_storage addr;
					socklen_t len = sizeof(addr);
					int error = 0;
					socklen_t errlen = sizeof(error);

					if(getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen) < 0) {

						error = errno;

					} else if (getpeername(sock, (struct sockaddr *)&addr, &len) == -1) {

						error = errno;

					} else if(Logger::enabled(Logger::Trace)) {

						char host[NI_MAXHOST];
						if (getnameinfo((struct sockaddr *) &addr, sizeof(addr), host, sizeof(host), NULL, 0, NI_NUMERICHOST) == 0) {
							Logger::String{"Connected to ",host}.trace();
						}

					}

					if(error) {
						::close(sock);
						errno = error;
						return -1;
					}


					return sock;
				}

			} else if(!mainloop) {
				::close(sock);
				errno = ECONNABORTED;
				return -1;

			}

			debug("Wait for ",(timeout - time(0))," seconds to connect to ",sock);	

		}

		debug("Timeout connecting to ",sock);
		::close(sock);

		errno = ETIMEDOUT;
		return -1;

	}

	void Socket::handle_event(const Event event) {

		try {

			if(event & MainLoop::Handler::onoutput) {

				if(connecting) {

					set((Event) (MainLoop::Handler::oninput|MainLoop::Handler::onhangup|MainLoop::Handler::onerror));
					connecting = false;

					struct sockaddr_storage addr;
					socklen_t len = sizeof(addr);

					int error = 0;
					socklen_t errlen = sizeof(error);

					if(getsockopt(values.fd, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen) < 0) {

						error = errno;

					} else if (getpeername(values.fd, (struct sockaddr *)&addr, &len) == -1) {

						error = errno;

					} else if(Logger::enabled(Logger::Trace)) {

						char host[NI_MAXHOST];
						if (getnameinfo((struct sockaddr *) &addr, sizeof(addr), host, sizeof(host), NULL, 0, NI_NUMERICHOST) == 0) {
							Logger::String{"Connected to ",host}.trace();
						}

					}

					if(error) {
						disable();
						close();
					}

					handle_connect(error);

				} else {

					handle_write_ok();

				}
				return;
			}

			if(event & MainLoop::Handler::onerror) {
				int error = ETIMEDOUT;
				socklen_t errlen = sizeof(error);
				if(getsockopt(values.fd, SOL_SOCKET, SO_ERROR, (void *)&error, &errlen) < 0) {
					error = errno;
				}
				disable();
				close();
				handle_error(error);
				return;
			}

			if(event & MainLoop::Handler::onhangup) {
				disable();
				close();
				handle_disconnect();
				return;
			}

		} catch(...) {

			close();
			throw;

		}

	}

	void Socket::handle_connect(int error) {
	}

	void Socket::handle_disconnect() {
	}

	void Socket::handle_read_ok() {
	}

	void Socket::handle_write_ok() {
	}

	void Socket::handle_error(int) {
	}

 }

