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
 #include <udjat/tools/net/nic.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <stdexcept>
 #include <cstring>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	class UDJAT_PRIVATE IfAddr : public ::ifaddrs, public Network::Interface {
	public:

		constexpr IfAddr(const ifaddrs &intf) : ifaddrs{intf} {
		}

		bool operator==(const sockaddr_storage &addr) const noexcept override {

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

		const char * name() const noexcept  {
			return ifa_name;
		}

		// Flags
		//
		// IFF_UP			Interface is running.
		// IFF_BROADCAST	Valid broadcast address set.
		// IFF_DEBUG		Internal debugging flag.
		// IFF_LOOPBACK		Interface is a loopback interface.
		// IFF_POINTOPOINT	Interface is a point-to-point link.
		// IFF_RUNNING		Resources allocated.
		// IFF_NOARP		No arp protocol, L2 destination address not
		//					set.
		// IFF_PROMISC		Interface is in promiscuous mode.
		// IFF_NOTRAILERS	Avoid use of trailers.
		// IFF_ALLMULTI		Receive all multicast packets.
		// IFF_MASTER		Master of a load balancing bundle.
		// IFF_SLAVE		Slave of a load balancing bundle.
		// IFF_MULTICAST	Supports multicast
		// IFF_PORTSEL		Is able to select media type via ifmap.
		// IFF_AUTOMEDIA	Auto media selection active.
		// IFF_DYNAMIC		The addresses are lost when the interface
		//					goes down.
		// IFF_LOWER_UP		Driver signals L1 up (since Linux 2.6.17)
		// IFF_DORMANT		Driver signals dormant (since Linux 2.6.17)
		// IFF_ECHO			Echo sent packets (since Linux 2.6.25)
		//

		bool up() const noexcept override {
			return ifa_flags & IFF_UP;
		}

		bool loopback() const noexcept override {
			return ifa_flags & IFF_LOOPBACK;
		}

	} ;

	bool Network::Interface::for_each(const std::function<bool(const Network::Interface &intf)> &func) {

		bool rc = false;

		struct ifaddrs * interfaces = nullptr;
		if(getifaddrs(&interfaces) != 0) {
			throw system_error(errno,system_category(),"Cant get network interfaces");
		}

		try {

			for(auto *interface = interfaces; interface && !rc; interface = interface->ifa_next) {
				rc = func(IfAddr{*interface});
			}

		} catch(...) {

			freeifaddrs(interfaces);
			throw;

		}

		freeifaddrs(interfaces);

		return rc;

	}


 }
