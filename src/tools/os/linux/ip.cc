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
 #include <udjat/net/ip/address.h>
 #include <udjat/linux/network.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <stdexcept>
 #include <cstring>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	sockaddr_storage IP::Factory(const char *addr) {

		sockaddr_storage storage;
		memset(&storage,0,sizeof(storage));

		if(addr && *addr) {
			if(inet_pton(AF_INET,addr,&((sockaddr_in *) &storage)->sin_addr) != 0) {
				storage.ss_family = AF_INET;
			} else if(inet_pton(AF_INET6,addr,&((sockaddr_in6 *) &storage)->sin6_addr) != 0) {
				storage.ss_family = AF_INET6;
			} else {
				throw std::system_error(errno, std::system_category(), addr);
			}
		}

		return storage;

	}

	bool IP::for_each(const std::function<bool(const IP::Addresses &addr)> &func) {

		return for_each([&func](const ifaddrs &intf){

			IP::Addresses addr;

			addr.interface_name = intf.ifa_name;
			addr.address = intf.ifa_addr;
			addr.netmask = intf.ifa_netmask;

			return func(addr);

		});

	}

 }

 namespace std {

 	UDJAT_API string to_string(const sockaddr_in &addr, bool dns) {

 		char host[NI_MAXHOST];
		memset(host,0,sizeof(host));

		int rc = getnameinfo(
					(sockaddr *) &addr,
					sizeof(struct sockaddr_in),
					host, NI_MAXHOST,
					NULL, 0,
					(dns ? 0 : NI_NUMERICHOST)
				);

		if(rc != 0) {
			throw runtime_error(gai_strerror(rc));
		}

		return string{host};

 	}

	UDJAT_API string to_string(const in_addr &addr, bool dns) {

		sockaddr_in ip;
		memset(&ip,0,sizeof(ip));
		ip.sin_family = AF_INET;
		ip.sin_addr = addr;

		return to_string(ip,dns);

	}

 	UDJAT_API string to_string(const sockaddr &addr, bool dns) {

  		char host[NI_MAXHOST];
		memset(host,0,sizeof(host));

		size_t sz;
		switch(addr.sa_family) {
		case AF_INET:
			sz = sizeof(sockaddr_in);
			break;

		case AF_INET6:
			sz = sizeof(sockaddr_in6);
			break;

		default:
			throw std::system_error(EINVAL, std::system_category(), "address family");
		}

		int rc = getnameinfo(
					&addr,
					sz,
					host, NI_MAXHOST,
					NULL, 0,
					(dns ? 0 : NI_NUMERICHOST)
				);

		if(rc != 0) {
			throw runtime_error(gai_strerror(rc));
		}

		return string{host};

	}


 	UDJAT_API string to_string(const sockaddr_in6 &addr, bool dns) {

 		char host[NI_MAXHOST];
		memset(host,0,sizeof(host));

		int rc = getnameinfo(
					(sockaddr *) &addr,
					sizeof(struct sockaddr_in6),
					host, NI_MAXHOST,
					NULL, 0,
					(dns ? 0 : NI_NUMERICHOST)
				);

		if(rc != 0) {
			throw runtime_error(gai_strerror(rc));
		}

		return string{host};

 	}

 }

