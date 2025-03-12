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
 #include <udjat/net/interface.h>
 #include <udjat/linux/network.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/value.h>
 #include <udjat/net/ip/address.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <net/if.h>
 #include <sys/ioctl.h>
 #include <stdexcept>
 #include <cstring>
 #include <iostream>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

	UDJAT_API bool IP::for_each(const std::function<bool(const ifaddrs &intf)> &func) {

		bool rc = false;

		struct ifaddrs * interfaces = nullptr;
		if(getifaddrs(&interfaces) != 0) {
			throw system_error(errno,system_category(),"Cant get network interfaces");
		}

		try {

			for(auto *interface = interfaces; interface && !rc; interface = interface->ifa_next) {
				rc = func(*interface);
			}

		} catch(...) {

			freeifaddrs(interfaces);
			throw;

		}

		freeifaddrs(interfaces);

		return rc;

	}

	bool Network::Interface::for_each(const std::function<bool(const char *name)> &func) {

		bool rc = false;

		// https://stackoverflow.com/questions/19227781/linux-getting-all-network-interface-names
		struct if_nameindex *if_nidxs;

		if_nidxs = if_nameindex();
		if(!if_nidxs) {
			throw std::system_error(errno,std::system_category());
		}

		try {

			for(struct if_nameindex *intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
				if(func(intf->if_name)) {
					rc = true;
					break;
				}
			}

		} catch(...) {

			if_freenameindex(if_nidxs);
			throw;

		}

		if_freenameindex(if_nidxs);
		return rc;

    }

   	bool Network::Interface::for_each(const std::function<bool(const Network::Interface &intf)> &func) {

		bool rc = false;

		// https://stackoverflow.com/questions/19227781/linux-getting-all-network-interface-names
		struct if_nameindex *if_nidxs;

		if_nidxs = if_nameindex();
		if(!if_nidxs) {
			throw std::system_error(errno,std::system_category());
		}

		try {

			for(struct if_nameindex *intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
				if(func(*Factory(intf->if_name))) {
					rc = true;
					break;
				}
			}

		} catch(...) {

			if_freenameindex(if_nidxs);
			throw;

		}

		if_freenameindex(if_nidxs);
		return rc;

	}

	unsigned int flags_by_name(const char *name) {

		int sock = socket(PF_INET, SOCK_STREAM, 0);
		if(sock < 0) {
			throw system_error(errno,system_category(),"Cant get PF_INET socket");
		}

		struct ifreq ifr;
		memset(&ifr,0,sizeof(ifr));
		strncpy(ifr.ifr_name,name,sizeof(ifr.ifr_name)-1);

		if(ioctl(sock, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
			int err = errno;
			::close(sock);
			throw system_error(err,system_category(),"SIOCGIFFLAGS");
		}

		unsigned int rc = ifr.ifr_flags;

		::close(sock);

		return rc;
	}

	std::shared_ptr<Network::Interface> Network::Interface::Factory(const char *name) {

		class NamedInterface : public Network::Interface {
		private:
			std::string nicname;

		public:
			NamedInterface(const char *name) : nicname{name} {
			}

			bool operator==(const sockaddr_storage &addr) const override {
				throw system_error(ENOTSUP,system_category(),"Unsupported method call");
			}

			bool found() const {
				return for_each([this](const char *intf) {
					return strcmp(intf,name()) == 0;
				});
			}

			const char * name() const override {
				return nicname.c_str();
			}

			bool up() const override {
				return flags_by_name(nicname.c_str()) & IFF_UP;
			}

			bool loopback() const override {
				return flags_by_name(nicname.c_str()) & IFF_LOOPBACK;
			}

			IP::Address address() const override {
				throw system_error(ENOTSUP,system_category(),"Cant get IP address on linux");
			}

			IP::Address netmask() const override {
				throw system_error(ENOTSUP,system_category(),"Cant get netmask on linux");
			}

			std::string macaddress() const override {
				
				int sock = socket(PF_INET, SOCK_STREAM, 0);
				if(sock < 0) {
					throw system_error(errno,system_category(),"Cant get PF_INET socket");
				}

				struct ifreq ifr;
				memset(&ifr,0,sizeof(ifr));
				strncpy(ifr.ifr_name,nicname.c_str(),sizeof(ifr.ifr_name)-1);

				if(ioctl(sock, SIOCGIFHWADDR, (caddr_t)&ifr) < 0) {
					int err = errno;
					::close(sock);
					throw system_error(err,system_category(),"SIOCGIFHWADDR");
				}

				::close(sock);

				std::string mac;
				static const char *digits = "0123456789ABCDEF";
				for(size_t ix = 0; ix < 6; ix++) {
					uint8_t digit = * (((unsigned char *) ifr.ifr_hwaddr.sa_data+ix));
					mac += digits[(digit >> 4) & 0x0f];
					mac += digits[digit & 0x0f];
				}

				return mac;
			}

		};

		return make_shared<NamedInterface>(name);
	}

 }
