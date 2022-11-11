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

 using namespace std;

 namespace Udjat {

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

	void Protocol::Worker::getmac(const sockaddr_storage &addr, std::string &mac) {

		string IpAddress{to_string(addr,false)};

		Win32::for_each([&IpAddress,&mac](const IP_ADAPTER_INFO &adapter) {

			if(!strcasecmp(IpAddress.c_str(),adapter.IpAddressList.IpAddress.String)) {

				mac.clear();

				static const char *digits = "0123456789ABCDEF";
				for(UINT ix = 0; ix < adapter.AddressLength; ix++) {
					uint8_t digit = ((unsigned char) adapter.Address[ix]);
					mac += digits[(digit >> 4) & 0x0f];
					mac += digits[digit & 0x0f];

				}

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


	}

 }
