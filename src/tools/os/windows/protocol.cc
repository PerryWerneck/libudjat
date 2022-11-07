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
 #include <udjat/tools/logger.h>
 #include <ws2tcpip.h>
 #include <udjat/win32/exception.h>
 #include <stdexcept>
 #include <sstream>
 #include <iomanip>

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

	/// @brief Get local address.
	static void getaddr(int sock, sockaddr_storage &addr) {

		int namelen = sizeof(addr);

		// https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-getpeername
		if(getsockname((SOCKET) sock, (sockaddr *) &addr, &namelen)) {
			throw Win32::WSA::Exception("Cant get peer name");
		}

	}

	void Protocol::Worker::getnic(const sockaddr_storage &addr, std::string &nic) {

		string IpAddress{to_string(addr)};

		Win32::for_each([&IpAddress,&nic](const IP_ADAPTER_INFO &adapter) {

			debug(adapter.AdapterName," ",adapter.IpAddressList.IpAddress.String);

			if(!strcasecmp(IpAddress.c_str(),adapter.IpAddressList.IpAddress.String)) {
				nic = adapter.AdapterName;
				return true;
			}

			return false;

		});


	}

	void Protocol::Worker::set_socket(int sock) {

		// Ignore if no payload.
		if(out.payload.empty()) {
			return;
		}

		{
			sockaddr_storage name;
			int namelen;

			// https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-getsockname
			namelen = sizeof(name);
			if(getsockname((SOCKET) sock, (sockaddr *) &name, &namelen)) {
				error() << "Cant get socket name" << endl;
			} else {
				set_local(name);
			}

			// https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-getpeername
			namelen = sizeof(name);
			if(getpeername((SOCKET) sock, (sockaddr *) &name, &namelen)) {
				error() << "Cant get peer name" << endl;
			} else {
				set_remote(name);
			}

		}

		out.payload.expand([this,sock](const char *key, std::string &value){

			if(strcasecmp(key,"macaddress") == 0) {

				sockaddr_storage addr;
				getaddr(sock,addr);

				string IpAddress{to_string(addr)};

				Win32::for_each([&IpAddress,&value](const IP_ADAPTER_INFO &adapter) {

					if(!strcasecmp(IpAddress.c_str(),adapter.IpAddressList.IpAddress.String)) {

						stringstream mac;

						for(UINT ix = 0; ix < adapter.AddressLength; ix++) {
							mac << setfill('0') << setw(2) << hex << ((int) adapter.Address[ix]) << dec;
						}

						value = mac.str();

						return true;
					}

					return false;

				});

			}

			return false;

		},false,false);

		/*
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

			if(strcasecmp(key,"network-interface") == 0) {
				getnic(sock,value);
				return true;
			}

			if(strcasecmp(key,"macaddress") == 0) {

				sockaddr_storage addr;
				getpeer(sock,addr);

				string IpAddress{to_string(addr)};

				Win32::for_each([&IpAddress,&value](const IP_ADAPTER_INFO &adapter) {

					if(!strcasecmp(IpAddress.c_str(),adapter.IpAddressList.IpAddress.String)) {

						stringstream mac;

						for(UINT ix = 0; ix < adapter.AddressLength; ix++) {
							mac << setfill('0') << setw(2) << hex << ((int) adapter.Address[ix]) << dec;
						}

						value = mac.str();

						return true;
					}

					return false;

				});

			}

			return false;

		},true,true);
		*/

	}

 }
