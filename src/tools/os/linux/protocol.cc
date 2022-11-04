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
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/ip.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <stdexcept>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <ifaddrs.h>
 #include <udjat/tools/logger.h>
 #include <iostream>
 #include <linux/if.h>
 #include <sys/ioctl.h>
 #include <sstream>
 #include <iomanip>

 using namespace std;

 namespace Udjat {

	UDJAT_API string to_string(const sockaddr_storage &addr) {

		if(!addr.ss_family) {
			throw runtime_error("Invalid IP address (no family)");
		}

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

	}

	static void getpeer(int sock, sockaddr_storage &addr) {
		socklen_t length = sizeof(addr);
		if(getpeername(sock, (sockaddr *) &addr, &length)) {
			throw system_error(errno,system_category(),"Cant get peer name");
		}
	}

	static void getnic(int sock, std::string &nic) {

		sockaddr_storage addr;
		getpeer(sock,addr);

		// Search for interfaces.
		struct ifaddrs * interfaces = nullptr;
		if(getifaddrs(&interfaces) != 0) {
			throw system_error(errno,system_category(),"Cant get network interfaces");
		}

		nic.clear(); // Just in case.

		try {

			for(auto *interface = interfaces; interface && nic.empty(); interface = interface->ifa_next) {

				if(interface->ifa_addr->sa_family != addr.ss_family) {
					continue;
				}

				debug("Testing interface ",interface->ifa_name);

				switch(interface->ifa_addr->sa_family) {
				case AF_INET:
					if( ((const struct sockaddr_in *) &addr)->sin_addr.s_addr == ((const struct sockaddr_in *) interface->ifa_addr)->sin_addr.s_addr) {
						nic = interface->ifa_name;
					}
					break;

				case AF_INET6:
					if( ((const struct sockaddr_in6 *) &addr)->sin6_addr.s6_addr == ((const struct sockaddr_in6 *) interface->ifa_addr)->sin6_addr.s6_addr) {
						nic = interface->ifa_name;
					}
					break;
				}

			}

		} catch(...) {

			freeifaddrs(interfaces);
			throw;

		}

		freeifaddrs(interfaces);

	}

	void Protocol::Worker::set_socket(int sock) {

		debug("*************************************");

		out.payload.expand([sock](const char *key, std::string &value){

			if(strcasecmp(key,"ipaddr") == 0) {

				sockaddr_storage addr;
				socklen_t length = sizeof(addr);

				if(getsockname(sock, (sockaddr *) &addr, &length)) {
					throw system_error(errno,system_category(),"Cant resolve ${ipaddr}");
				}

				value = to_string(addr);

				return true;

			}

			if(strcasecmp(key,"hostip") == 0) {

				sockaddr_storage addr;
				getpeer(sock,addr);
				value = to_string(addr);

				return true;
			}

			if(strcasecmp(key,"network-interface") == 0) {
				getnic(sock,value);
				return true;
			}

			if(strcasecmp(key,"macaddress") == 0) {

				string nic;
				getnic(sock,nic);

				struct ifreq ifr;

				memset(&ifr,0,sizeof(ifr));
				strncpy(ifr.ifr_name, nic.c_str(), sizeof( ifr.ifr_name ) );

				if(ioctl(sock, SIOCGIFHWADDR, &ifr) < 0 ) {
					cerr << "protocol\tCant get mac address for '" << nic << "': " << strerror(errno) << endl;
					value.clear();
					return false;
				}

				value.clear();
				static const char *digits = "0123456789ABCDEF";
				for(size_t ix = 0; ix < 6; ix++) {
					uint8_t digit = * (((unsigned char *) ifr.ifr_hwaddr.sa_data+ix));

					value += digits[(digit >> 4) & 0x0f];
					value += digits[digit & 0x0f];

				}

				return true;
			}

			return false;

		},true,true);

		debug("Payload:\n",out.payload);

	}


 }
