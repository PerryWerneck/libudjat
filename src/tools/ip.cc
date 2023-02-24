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
 #include <udjat/net/ip/address.h>
 #include <stdexcept>
 #include <cstring>
 #include <udjat/tools/logger.h>
 #include <netpacket/packet.h>

 using namespace std;

 namespace Udjat {

	UDJAT_API sockaddr_storage IP::Factory(const sockaddr_in *addr) {
		sockaddr_storage result;
		memset(&result,0,sizeof(result));

		if(addr) {
			memcpy(&result,addr,sizeof(sockaddr_in));
		}

		result.ss_family = AF_INET;
		return result;
	}

	UDJAT_API sockaddr_storage IP::Factory(const sockaddr_in6 *addr) {
		sockaddr_storage result;
		memset(&result,0,sizeof(result));

		if(addr) {
			memcpy(&result,addr,sizeof(sockaddr_in6));
		}

		result.ss_family = AF_INET6;
		return result;
	}

	UDJAT_API sockaddr_storage IP::Factory(const sockaddr *addr) {

		if(!addr) {
			sockaddr_storage result;
			memset(&result,0,sizeof(result));
			return result;
		}

		switch(addr->sa_family) {
		case AF_INET:
			return Factory((const sockaddr_in *) addr);

		case AF_INET6:
			return Factory((const sockaddr_in6 *) addr);

		case AF_PACKET:
			{
				sockaddr_storage result;
				memset(&result,0,sizeof(result));

				if(addr) {
					memcpy(&result,addr,sizeof(sockaddr_ll));
				}

				result.ss_family = AF_PACKET;
				return result;
			}

		default:
			throw runtime_error(Logger::Message{"Dont know how to factory an IP::Address for family '{}'",(int) addr->sa_family});

		}

	}

	UDJAT_API sockaddr_storage IP::Factory(const pugi::xml_node &node) {
		return Factory((const char *) node.attribute("ip").as_string());
	}

	IP::Address & IP::Address::set(const sockaddr_storage & value) {
		*((sockaddr_storage *) this) = value;
		return *this;
	}

	bool IP::Address::equal(const sockaddr_storage &a, const sockaddr_storage &b, bool port) {

		if(a.ss_family != b.ss_family) {
			return false;
		}

		switch(a.ss_family) {
		case 0:
			return true;

		case AF_INET:
			if(port && ((sockaddr_in *) &a)->sin_port != ((sockaddr_in *) &b)->sin_port ) {
				debug("a.port=",((sockaddr_in *) &a)->sin_port," b.port=",((sockaddr_in *) &b)->sin_port)
				return false;
			}
			if( ((sockaddr_in *) &a)->sin_addr.s_addr != ((sockaddr_in *) &b)->sin_addr.s_addr ) {
				return false;
			}
			break;

		case AF_INET6:
			if(port && ((sockaddr_in6 *) &a)->sin6_port != ((sockaddr_in6 *) &b)->sin6_port ) {
				return false;
			}
			if( memcmp( &(((sockaddr_in6 *) &a)->sin6_addr), &(((sockaddr_in6 *) &b)->sin6_addr), sizeof(((sockaddr_in6 *) &b)->sin6_addr) ) ) {
				return false;
			}
			break;

		default:
			throw runtime_error(Logger::Message{"Unexpected address family '{}'",((int) a.ss_family)});

		}

		return true;

	}

	std::string IP::Address::to_string() const noexcept {
		if(empty()) {
			return "";
		}
		return std::to_string((sockaddr_storage) *this);
	}


 }

 namespace std {

	UDJAT_API string to_string(const sockaddr_storage &addr, bool dns) {

		switch(addr.ss_family) {
		case AF_INET:
			return to_string( *((sockaddr_in *) &addr), dns);

		case AF_INET6:
			return to_string( *((sockaddr_in6 *) &addr), dns);

		}

		return "";

	}

 }

