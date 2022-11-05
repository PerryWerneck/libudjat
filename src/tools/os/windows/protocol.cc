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
 #include <udjat/win32/ip.h>
 #include <ws2tcpip.h>
 #include <udjat/win32/exception.h>
 #include <stdexcept>

 using namespace std;

 namespace std {

	UDJAT_API string to_string(const sockaddr_storage &addr) {

		if(!addr.ss_family) {
			throw runtime_error("Invalid IP address (no family)");
		}

		char ipaddr[256];
		memset(ipaddr,0,sizeof(ipaddr));

		switch(addr.ss_family) {
		case AF_INET:
			InetNtop(
				((const struct sockaddr_in *) &addr)->sin_family,
				&((const struct sockaddr_in *) &addr)->sin_addr,
				ipaddr,
				sizeof(ipaddr)
			);
			break;

		case AF_INET6:
			InetNtop(
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

 }

 namespace Udjat {

	static void getpeer(int sock, sockaddr_storage &addr) {

		int namelen = sizeof(addr);

		// https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-getpeername
		if(getpeername((SOCKET) sock, (sockaddr *) &addr, &namelen)) {
			throw Win32::WSA::Exception("Cant get peer name");
		}

	}

	static void getnic(int sock, std::string &nic) {

		sockaddr_storage addr;
		getpeer(sock,addr);

	}

	void Protocol::Worker::set_socket(int sock) {

		out.payload.expand([sock](const char *key, std::string &value){

			if(strcasecmp(key,"ipaddr") == 0) {

				// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-getsockname

				sockaddr_storage name;
				int namelen = sizeof(name);

				if(getsockname((SOCKET) sock, (sockaddr *) &name, &namelen)) {
					throw Win32::WSA::Exception("Cant resolve ${ipaddr}");
				}

				value = std::to_string(name);

				return true;

			}

			if(strcasecmp(key,"hostip") == 0) {

				sockaddr_storage addr;
				getpeer(sock,addr);
				value = to_string(addr);

				return true;

			}

			return false;

		},true,true);

	}

 }
