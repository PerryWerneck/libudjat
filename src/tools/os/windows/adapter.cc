/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/win32/ip.h>
 #include <udjat/net/ip/address.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/logger.h>
 #include <malloc.h>
 #include <udjat/win32/container.h>

 // #pragma comment(lib, "iphlpapi.lib")

 namespace Udjat {

	UDJAT_API bool Win32::for_each(const std::function<bool(const IP_ADAPTER_ADDRESSES &address)> &func) {

		class Addresses : public Win32::Container<IP_ADAPTER_ADDRESSES> {
		protected:
			DWORD load(IP_ADAPTER_ADDRESSES *buffer, ULONG *ifbuffersize) override {
				return GetAdaptersAddresses(AF_INET,0,0,buffer,ifbuffersize);
			}
		} addresses;

		for(const IP_ADAPTER_ADDRESSES *address = addresses.get();address;address = address->Next) {
			if(func(*address)) {
				return true;
			}
		}

		return false;

	}


	UDJAT_API bool Win32::for_each(const std::function<bool(const IP_ADAPTER_INFO &info)> &func) {

		class Infos : public Win32::Container<IP_ADAPTER_INFO> {
		protected:
			DWORD load(IP_ADAPTER_INFO *buffer, ULONG *ifbuffersize) override {
				return GetAdaptersInfo(buffer,ifbuffersize);
			}
		} infos;

		for(const IP_ADAPTER_INFO *info = infos.get();info;info = info->Next) {
			if(func(*info)) {
				return true;
			}
		}

		return false;

	}

 }
