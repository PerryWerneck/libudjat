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
 #include <udjat/tools/ip.h>

 using namespace std;

 namespace std {

 	UDJAT_API string to_string(const sockaddr_in &addr, bool UDJAT_UNUSED(dns)) {

		char ipaddr[256];
		memset(ipaddr,0,sizeof(ipaddr));

		InetNtop(
			addr.sin_family,
			&addr.sin_addr,
			ipaddr,
			sizeof(ipaddr)
		);

		return string{ipaddr};

 	}

 	UDJAT_API string to_string(const sockaddr_in6 &addr, bool UDJAT_UNUSED(dns)) {

		char ipaddr[256];
		memset(ipaddr,0,sizeof(ipaddr));

		InetNtop(
			addr.sin6_family,
			&addr.sin6_addr,
			ipaddr,
			sizeof(ipaddr)
		);

		return string{ipaddr};

 	}

 }

