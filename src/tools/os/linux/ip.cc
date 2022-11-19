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
 #include <udjat/linux/network.h>
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

	}

 }

 namespace Udjat {

	bool Network::Interface::operator==(const sockaddr_storage &addr) const noexcept {

		if(!ifa_addr || (ifa_addr->sa_family != addr.ss_family)) {
			return false;
		}

		switch(ifa_addr->sa_family) {
		case AF_INET:
			if( ((const struct sockaddr_in *) &addr)->sin_addr.s_addr == ((const struct sockaddr_in *) ifa_addr)->sin_addr.s_addr) {
				return true;
			}
			break;

		case AF_INET6:
			if( ((const struct sockaddr_in6 *) &addr)->sin6_addr.s6_addr == ((const struct sockaddr_in6 *) ifa_addr)->sin6_addr.s6_addr) {
				return true;
			}
			break;
		}

		return false;

	}

	bool for_each(const std::function<bool(const Network::Interface &intf)> &func) {

		bool rc = false;

		struct ifaddrs * interfaces = nullptr;
		if(getifaddrs(&interfaces) != 0) {
			throw system_error(errno,system_category(),"Cant get network interfaces");
		}

		try {

			for(auto *interface = interfaces; interface && !rc; interface = interface->ifa_next) {
				rc = func(Network::Interface{*interface});
			}

		} catch(...) {

			freeifaddrs(interfaces);
			throw;

		}

		freeifaddrs(interfaces);

		return rc;

	}


 }
