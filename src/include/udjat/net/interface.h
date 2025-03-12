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

 #pragma once

 #include <udjat/defs.h>
 #include <string>
 #include <cstring>
 #include <functional>
 #include <memory>
 #include <udjat/net/ip/address.h>

 #ifndef _WIN32
	#include <sys/socket.h>
 #endif // _WIN32

 namespace Udjat {

 	namespace Network {

		/// @brief Network Interface.
		class UDJAT_API Interface {
		public:

			virtual bool operator==(const sockaddr_storage &addr) const = 0;

			inline bool operator==(const char *str) const {
				return strcasecmp(name(),str) == 0;
			}

			virtual const char * name() const = 0;

			virtual Value & getProperties(Value &value) const;

			virtual bool found() const = 0;
			virtual bool up() const = 0;
			virtual bool loopback() const = 0;
			virtual std::string macaddress() const = 0;
			virtual IP::Address address() const = 0;			///< @brief Interface address.
			virtual IP::Address netmask() const = 0;			///< @brief Interface netmask.

			/// @brief Build interface from nic name.
			static std::shared_ptr<Interface> Factory(const char *name);

			/// @brief Enumerate all interface names.
			static bool for_each(const std::function<bool(const char *name)> &func);

			/// @brief Enumerate all interfaces.
			static bool for_each(const std::function<bool(const Network::Interface &intf)> &func);

		};

 	}

	/// @return true if 'func' has returned true.
	//UDJAT_API bool for_each(const std::function<bool(const Network::Interface &intf)> &func);

 }
