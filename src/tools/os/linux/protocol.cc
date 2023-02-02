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
 #include <udjat/tools/net/ip.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <stdexcept>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <ifaddrs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/net/nic.h>
 #include <iostream>
 #include <linux/if.h>
 #include <sys/ioctl.h>
 #include <sstream>
 #include <iomanip>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

	void Protocol::Worker::getnic(const sockaddr_storage &addr, std::string &nic) {

		nic.clear(); // Just in case.

		Network::Interface::for_each([&nic,&addr](const Network::Interface &interface){

			if(interface == addr) {
				nic = interface.name();
				return true;
			}

			return false;

		});

	}

	void Protocol::Worker::getmac(const sockaddr_storage &addr, std::string &mac) {

		// Get NIC using addr.
		string nic;
		getnic(addr,nic);

		// Get mac address from NIC
		struct ifreq ifr;

		memset(&ifr,0,sizeof(ifr));
		strncpy(ifr.ifr_name, nic.c_str(), sizeof( ifr.ifr_name ) );

		mac.clear();

		{
			int sock = socket(PF_INET, SOCK_STREAM, 0);
			int rc = ioctl(sock, SIOCGIFHWADDR, &ifr);
			::close(sock);
			if(rc < 0 ) {
				error() << "Cant get mac address for '" << nic << "': " << strerror(errno) << endl;
				return;
			}
		}

		static const char *digits = "0123456789ABCDEF";
		for(size_t ix = 0; ix < 6; ix++) {
			uint8_t digit = * (((unsigned char *) ifr.ifr_hwaddr.sa_data+ix));
			mac += digits[(digit >> 4) & 0x0f];
			mac += digits[digit & 0x0f];
		}

	}

	void Protocol::Worker::set_socket(int sock) {

		// Ignore if no payload.
		if(out.payload.empty()) {
			return;
		}

		sockaddr_storage addr;
		socklen_t length;

		length = sizeof(addr);
		if(!getsockname(sock, (sockaddr *) &addr, &length)) {
			set_local(addr);
		}

		length = sizeof(addr);
		if(!getpeername(sock, (sockaddr *) &addr, &length)) {
			set_remote(addr);
		}

	}

	const char * Protocol::Worker::get_payload() noexcept {
		out.payload.expand(true,true);
		return out.payload.c_str();
	}

 }
