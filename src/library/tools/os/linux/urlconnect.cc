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
 #include <udjat/tools/exception.h>
 #include <string>
 #include <stdexcept>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <unistd.h>
 #include <stdexcept>
 #include <fcntl.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/socket.h>
 #include <poll.h>

 using namespace std;

 namespace Udjat {

	int URL::connect(unsigned int msec) {

		int rc;

		struct pollfd pfd;
		memset(&pfd,0,sizeof(pfd));

		if(msec < 1) {
			msec = Config::Value<unsigned int>("network","timeout",5000).get();
		}

        struct addrinfo   hints;
        struct addrinfo * result        = NULL;
        memset(&hints,0,sizeof(hints));

        hints.ai_family         = AF_UNSPEC;    // Allow IPv4 or IPv6
        hints.ai_socktype       = SOCK_STREAM;  // Stream socket
        hints.ai_flags          = AI_PASSIVE;   // For wildcard IP address
        hints.ai_protocol       = 0;                    // Any protocol

        rc = getaddrinfo(hostname().c_str(), servicename().c_str(), &hints, &result);
        if(rc) {
			throw Exception(rc, Logger::String{"Failed to resolve '",c_str(),"'"},gai_strerror(rc));
		}

		int error = 0;
		for(struct addrinfo *rp = result; rp; rp = rp->ai_next) {

			pfd.fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if(pfd.fd < 0) {
				continue;
			}

			Socket::blocking(pfd.fd,false);

			if(::connect(pfd.fd,rp->ai_addr, rp->ai_addrlen) && errno != EINPROGRESS) {
				error = errno;
				close(pfd.fd);
				pfd.fd = -1;
				continue;
			}

			break;
		}

		freeaddrinfo(result);

		if(pfd.fd < 0 && error > 0) {

			if(error > 0) {
				throw system_error(error,system_category(),Logger::String{"Failed to connect to '",c_str(),"'"});
			}
	
			throw runtime_error(Logger::String{"Failed to connect to '",c_str(),"'"});
		}

		pfd.events = POLLOUT;

		rc = poll(&pfd,1,msec);
		if(rc < 0) {
			throw system_error(errno,system_category(),Logger::String{"Failed to poll socket to '",c_str(),"'"});
		}

		if(rc == 0) {
			throw system_error(ETIMEDOUT,system_category(),Logger::String{"Timeout connecting to '",c_str(),"'"});
		}

		return pfd.fd;

	}


 }

