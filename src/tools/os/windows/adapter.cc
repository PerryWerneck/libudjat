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
 #include <udjat/tools/ip.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/logger.h>
 #include <malloc.h>

 // #pragma comment(lib, "iphlpapi.lib")

 namespace Udjat {

	UDJAT_API bool Win32::for_each(const std::function<bool(const IP_ADAPTER_ADDRESSES &address)> &func) {

		bool found = false;

		ULONG ifbuffersize = sizeof(IP_ADAPTER_ADDRESSES)*2;

		PIP_ADAPTER_ADDRESSES addresses = (PIP_ADAPTER_ADDRESSES) malloc(ifbuffersize+10);

		try {

			memset(addresses,0,ifbuffersize);
			DWORD rc = GetAdaptersAddresses(AF_INET,0,0,addresses,&ifbuffersize);

			if(rc == ERROR_INSUFFICIENT_BUFFER || rc == ERROR_BUFFER_OVERFLOW) {

				addresses = (PIP_ADAPTER_ADDRESSES) realloc(addresses,ifbuffersize+10);
				rc = GetAdaptersAddresses(AF_INET,0,0,addresses,&ifbuffersize);

			}

			if(rc != NO_ERROR) {
				throw Win32::Exception("GetAdaptersAddresses() has failed",rc);
			}

			for(PIP_ADAPTER_ADDRESSES address = addresses;address;address = address->Next) {
				if(func(*address)) {
					found = true;
					break;
				}
			}

		} catch(...) {

			free(addresses);
			throw;

		}

		free(addresses);
		return found;

	}


	UDJAT_API bool Win32::for_each(const std::function<bool(const IP_ADAPTER_INFO &info)> &func) {

		bool found = false;
		ULONG ifbuffersize = sizeof(IP_ADAPTER_INFO)*2;

		PIP_ADAPTER_INFO infos = (PIP_ADAPTER_INFO) malloc(ifbuffersize+10);

		try {

			memset(infos,0,ifbuffersize);
			DWORD rc = GetAdaptersInfo(infos,&ifbuffersize);

			if(rc == ERROR_INSUFFICIENT_BUFFER || rc == ERROR_BUFFER_OVERFLOW) {

				infos = (PIP_ADAPTER_INFO) realloc(infos,ifbuffersize+10);
				rc = GetAdaptersInfo(infos,&ifbuffersize);

			}

			if(rc != NO_ERROR) {
				throw Win32::Exception("GetAdaptersInfo() has failed",rc);
			}

			for(PIP_ADAPTER_INFO info = infos;info;info = info->Next) {
				if(func(*info)) {
					found = true;
					break;
				}
			}

		} catch(...) {

			free(infos);
			throw;

		}

		free(infos);

		return found;

	}

 }
