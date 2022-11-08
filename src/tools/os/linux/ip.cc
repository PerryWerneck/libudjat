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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/ip.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <stdexcept>
 #include <cstring>
 #include <iostream>

 using namespace std;

 namespace std {

	UDJAT_API string to_string(const sockaddr_storage &addr, bool dns) {

		if(addr.ss_family == AF_PACKET) {
			// TODO: How to get ipaddr on AF_PACKET?
			return "";
		}

		char host[NI_MAXHOST];
		memset(host,0,sizeof(host));

		int rc = getnameinfo(
					(sockaddr *) &addr,
					(addr.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)),
					host, NI_MAXHOST,
					NULL, 0,
					(dns ? 0 : NI_NUMERICHOST)
				);

		if(rc != 0) {
			throw runtime_error(gai_strerror(rc));
		}

		return string{host};

		/*
		char ipaddr[256];
		memset(ipaddr,0,sizeof(ipaddr));

		switch(addr.ss_family) {
		case AF_INET:
			inet_ntop(
				((const struct sockaddr_in *) &addr)->sin_family,
				&((const struct sockaddr_in *) &addr)->sin_addr,
				ipaddr,
				sizeof(ipaddr)
			);
			break;

		case AF_INET6:
			inet_ntop(
				((const struct sockaddr_in6 *) &addr)->sin6_family,
				&((const struct sockaddr_in6 *) &addr)->sin6_addr,
				ipaddr,
				sizeof(ipaddr)
			);
			break;

		default:
			throw runtime_error("Unexpected IPADDR family");

		}
		return string{ipaddr};
		*/


	}

 }

