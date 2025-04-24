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
 #include <system_error>

 using namespace std;

 /// @brief Simple socket wrapper.
 /// @details This class is used to create a socket and perform ioctl operations
 struct UDJAT_PRIVATE Socket {

	int fd;

	Socket(int domain = PF_INET, int type = SOCK_STREAM, int protocol = 0) : fd{socket(domain,type,protocol)} {
		if(fd < 0) {
			throw system_error(errno,system_category(),"Cant get PF_INET socket");
		}
	}

	~Socket() {
		::close(fd);
	}

	inline operator int() const noexcept {
		return fd;
	}

	template <typename T>
	inline void ioctl(unsigned long op, T &val) const {
		if(::ioctl(this->fd, op, (caddr_t)&val) < 0) {
			throw system_error(errno,system_category(),"ioctl error");
		}
	}

 };

 /// @brief Network interface implementation.
 /// @details This class is used to get network interface properties based on nicname.
 class UDJAT_PRIVATE NamedInterface : public Udjat::Network::Interface {
 private:

	void get(struct ifreq &ifr) const noexcept {
		memset(&ifr,0,sizeof(ifr));
		strncpy(ifr.ifr_name,nicname.c_str(),sizeof(ifr.ifr_name)-1);
	}

	std::string nicname;

	unsigned int flags() const {
		Socket sock;
		struct ifreq ifr;
		get(ifr);
		sock.ioctl(SIOCGIFFLAGS, ifr);
		return ifr.ifr_flags;
	 }
	
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
		return flags() & IFF_UP;
	}

	bool loopback() const override {
		return flags() & IFF_LOOPBACK;
	}

	Udjat::IP::Address address() const override {
		Socket sock;
		struct ifreq ifr;
		get(ifr);
		sock.ioctl(SIOCGIFADDR,ifr);
		return Udjat::IP::Address{&ifr.ifr_addr};
	}

	Udjat::IP::Address netmask() const override {
		Socket sock;
		struct ifreq ifr;
		get(ifr);
		sock.ioctl(SIOCGIFNETMASK,ifr);
		return Udjat::IP::Address{&ifr.ifr_netmask};
	}

	std::string macaddress() const override {
		
		Socket sock;

		struct ifreq ifr;
		get(ifr);

		sock.ioctl(SIOCGIFHWADDR,ifr);

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

	std::shared_ptr<Network::Interface> Network::Interface::Default() {

	}

	std::shared_ptr<Network::Interface> Network::Interface::Factory(const char *name) {
		return make_shared<NamedInterface>(name);
	}

 }
