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
 #include <stdexcept>
 #include <cstring>
 #include <iostream>
 #include <unistd.h>
 #include <iptypes.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/container.h>
// #include <udjat/win32/network.h>
 #include <iphlpapi.h>
 #include <udjat/net/ip/address.h>

 using namespace std;

 namespace Udjat {

	static std::string get_macaddress(const IP_ADAPTER_INFO *iface) {
		string rc;
		for (UINT i = 0; i < iface->AddressLength; i++) {
			char buffer[4];
			snprintf(buffer,3,"%02X",(int) iface->Address[i]);
			buffer[4] = 0;
			rc += buffer;
		}
		return rc;
	}

	class UDJAT_PRIVATE Interfaces : public Win32::Container<IP_ADAPTER_INFO> {
	protected:
		DWORD load(IP_ADAPTER_INFO *buffer, ULONG *ifbuffersize) override {
			return GetAdaptersInfo(buffer,ifbuffersize);
		}
	};

	bool Network::Interface::for_each(const std::function<bool(const char *name)> &func) {

		Interfaces interfaces;

		for(const IP_ADAPTER_INFO * iface = interfaces.get();iface;iface = iface->Next) {
			if(func(iface->AdapterName)) {
				return true;
			}
		}

		return false;

	}

	bool Network::Interface::for_each(const std::function<bool(const Network::Interface &intf)> &func) {

		class UDJAT_PRIVATE IfAddr : public Network::Interface {
		private:
			const IP_ADAPTER_INFO *iface;

		public:
			constexpr IfAddr(const IP_ADAPTER_INFO *i) : iface{i} {
			}

			const char * name() const override {
				return iface->AdapterName;
			}

			std::string macaddress() const override {
				return get_macaddress(iface);
			}

			bool operator==(const sockaddr_storage &addr) const override {
				return strcasecmp(std::to_string(addr).c_str(),(const char *) iface->CurrentIpAddress) == 0;
			}

			bool found() const override {
				return true;
			}

			bool up() const override {
				return true;
			}

			bool loopback() const override {
				return iface->Type == MIB_IF_TYPE_LOOPBACK;
			}

		};

		Interfaces interfaces;

		for(const IP_ADAPTER_INFO * iface = interfaces.get();iface;iface = iface->Next) {
			if(func(IfAddr{iface})) {
				return true;
			}
		}

		return false;

	}

	std::shared_ptr<Network::Interface> Network::Interface::Factory(const char *name) {

		class NamedInterface : public Network::Interface {
		private:
			std::string nicname;

			inline bool equal(const IP_ADAPTER_INFO * iface) const noexcept {
				return strcasecmp(nicname.c_str(),iface->AdapterName) == 0;
			}

		public:
			NamedInterface(const char *name) : nicname{name} {
			}

			bool operator==(const sockaddr_storage &addr) const override {
				throw system_error(ENOTSUP,system_category(),"Unsupported method call");
			}

			bool found() const {
				Interfaces interfaces;
				for(const IP_ADAPTER_INFO * iface = interfaces.get();iface;iface = iface->Next) {
					if(equal(iface)) {
						return true;
					}
				}
				return false;
			}

			const char * name() const override {
				return nicname.c_str();
			}

			bool up() const override {
				return true;
			}

			std::string macaddress() const override {
				Interfaces interfaces;
				for(const IP_ADAPTER_INFO * iface = interfaces.get();iface;iface = iface->Next) {
					if(equal(iface)) {
						return get_macaddress(iface);
					}
				}
				return "";
			}

			bool loopback() const override {
				Interfaces interfaces;
				for(const IP_ADAPTER_INFO * iface = interfaces.get();iface;iface = iface->Next) {
					if(equal(iface)) {
						return iface->Type == MIB_IF_TYPE_LOOPBACK;
					}
				}
				return false;
			}

		};

		return make_shared<NamedInterface>(name);

	}

 }
