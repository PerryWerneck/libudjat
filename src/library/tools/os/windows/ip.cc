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
 #include <udjat/net/interface.h>
 #include <ws2tcpip.h>
 #include <system_error>
 #include <udjat/win32/exception.h>

 using namespace std;

 namespace Udjat {

	sockaddr_storage IP::Factory(const char *addr) {

		sockaddr_storage storage;
		memset(&storage,0,sizeof(storage));

		if(addr && *addr) {
			if(InetPton(AF_INET,addr,&((sockaddr_in *) &storage)->sin_addr) != 0) {
				storage.ss_family = AF_INET;
			} else if(InetPton(AF_INET6,addr,&((sockaddr_in6 *) &storage)->sin6_addr) != 0) {
				storage.ss_family = AF_INET6;
			} else {
				throw Win32::WSA::Exception(addr);
			}
		}

		return storage;

	}

	bool IP::for_each(const std::function<bool(const IP::Addresses &addr)> &func) {

		return Network::Interface::for_each([&func](const Network::Interface &intf) {

			IP::Addresses addr;

			addr.interface_name = intf.name();
			addr.address = intf.address();
			addr.netmask = intf.netmask();

			return func(addr);

		});

	}

 }

 namespace std {

	UDJAT_API string to_string(const in_addr &addr, bool dns) {

		sockaddr_in ip;
		memset(&ip,0,sizeof(ip));
		ip.sin_family = AF_INET;
		ip.sin_addr = addr;

		return to_string(ip,dns);

	}

 	UDJAT_API string to_string(const sockaddr &addr, bool dns) {

 		char ipaddr[256];
		memset(ipaddr,0,sizeof(ipaddr));

		switch(addr.sa_family) {
		case AF_INET:
			InetNtop(
				((const sockaddr_in *) &addr)->sin_family,
				(void *) &(((const sockaddr_in *) &addr)->sin_addr),
				ipaddr,
				sizeof(sockaddr_in)
			);
			break;

		case AF_INET6:
			InetNtop(
				((const sockaddr_in6 *) &addr)->sin6_family,
				(void *) &(((const sockaddr_in6 *) &addr)->sin6_addr),
				ipaddr,
				sizeof(sockaddr_in6)
			);
			break;

		default:
			throw std::system_error(EINVAL, std::system_category(), "address family");
		}


		return string{ipaddr};

 	}

 	UDJAT_API string to_string(const sockaddr_in &addr, bool UDJAT_UNUSED(dns)) {

		char ipaddr[256];
		memset(ipaddr,0,sizeof(ipaddr));

		InetNtop(
			addr.sin_family,
			&addr.sin_addr,
			ipaddr,
			sizeof(ipaddr)
		);

		return string{ipaddr};

 	}

 	UDJAT_API string to_string(const sockaddr_in6 &addr, bool UDJAT_UNUSED(dns)) {

		char ipaddr[256];
		memset(ipaddr,0,sizeof(ipaddr));

		InetNtop(
			addr.sin6_family,
			&addr.sin6_addr,
			ipaddr,
			sizeof(ipaddr)
		);

		return string{ipaddr};

 	}

 }

