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
 #include <udjat/net/ip/address.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <stdexcept>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <ifaddrs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/net/interface.h>
 #include <iostream>
 #include <net/if.h>
 #include <sys/ioctl.h>
 #include <sstream>
 #include <iomanip>
 #include <unistd.h>
 #include <netpacket/packet.h>

 using namespace std;

 namespace Udjat {

	bool Protocol::Worker::getnic(const sockaddr_storage &addr, std::string &nic) {

		nic.clear(); // Just in case.

		if(addr.ss_family == AF_PACKET) {

			// AF_PACKET, get interface name from index.

			char name[IF_NAMESIZE+1];
			memset(name,0,IF_NAMESIZE+1);

			char *ifname = if_indextoname(((sockaddr_ll *) &addr)->sll_ifindex,name);
			if(!ifname) {
				cerr << "linux\tCant name interface '" << ((sockaddr_ll *) &addr)->sll_ifindex << "': " << strerror(errno) << endl;
				return false;
			}

			debug("Got packet interface '",ifname,"'");

			nic = ifname;
			return true;
		}

		return IP::for_each([&nic,&addr](const IP::Addresses &info){

			debug("a----------------> ",info.interface_name);
			debug(std::to_string(info.address)," - ",std::to_string(addr));

			if(info.address == addr) {
				nic = info.interface_name;
				debug("Found '",nic,"'");
				return true;
			}

			return false;

		});

	}

	void Protocol::Worker::getmac(const sockaddr_storage &addr, std::string &mac) {

		mac.clear();

		// Get NIC using addr.
		string nic;
		getnic(addr,nic);
		if(nic.empty()) {
			error() << "Cant identify interface for '" << addr << "'" << endl;
			return;
		}

		// Get mac address from NIC
		struct ifreq ifr;

		memset(&ifr,0,sizeof(ifr));
		strncpy(ifr.ifr_name, nic.c_str(), sizeof( ifr.ifr_name ) );

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
