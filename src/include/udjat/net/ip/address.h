/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/xml.h>
 #include <functional>
 #include <udjat/net/ip/address.h>

 #ifdef _WIN32
	#include <ws2ipdef.h>
 #else
	#include <sys/socket.h>
	#include <arpa/inet.h>
 #endif // _WIN32

 namespace Udjat {

	namespace IP {

		UDJAT_API sockaddr_storage Factory(const char *addr);
		UDJAT_API sockaddr_storage Factory(const sockaddr *addr);
		UDJAT_API sockaddr_storage Factory(const sockaddr_in *addr);
		UDJAT_API sockaddr_storage Factory(const sockaddr_in6 *addr);
		UDJAT_API sockaddr_storage Factory(const XML::Node &node);

		class UDJAT_API Address : public sockaddr_storage {
		private:

		public:

#if __cplusplus >= 201703L
			Address(const char *ip) : sockaddr_storage{IP::Factory(ip)} {
			}

			constexpr Address(const sockaddr_storage &a) : sockaddr_storage{a} {
			}

			constexpr Address() : sockaddr_storage{} {
			}

			template <typename T>
			Address(const T *addr) : sockaddr_storage{IP::Factory(addr)} {
			}
#else
			Address(const char *ip) {
				*((sockaddr_storage *) this) = IP::Factory(ip);
			}

			Address(const sockaddr_storage &a) {
				*((sockaddr_storage *) this) = a;
			}

			Address() {
			}

			template <typename T>
			Address(const T *addr) {
				*((sockaddr_storage *) this) = IP::Factory(addr);
			}
#endif

			/// @brief Compare 2 IP Addresses
			/// @param a First IP address to compare.
			/// @param b Second IP address to compare.
			/// @param port If true compare the port numbers.
			/// @retval true Same addresses (and port numbers).
			/// @retval false Not the same addresses (or port numbers).
			static bool equal(const sockaddr_storage &a, const sockaddr_storage &b, bool port = false);

			inline bool operator==(const sockaddr_storage &storage) const noexcept {
				return equal((sockaddr_storage) *this, storage);
			}

			inline bool operator==(const Address &addr) const noexcept {
				return equal((sockaddr_storage) *this, (sockaddr_storage) addr);
			}

			inline bool empty() const noexcept {
				return ss_family == 0;
			}

			inline operator bool() const noexcept {
				return ss_family != 0;
			}

			inline Address & clear() noexcept {
				ss_family = 0;
				return *this;
			}

			Address & set(const sockaddr_storage & value);

			inline Address & set(const std::string &value) {
				return set(value.c_str());
			}

			template <typename T>
			Address & set(const T * value) {
				*((sockaddr_storage *) this) = IP::Factory(value);
				return *this;
			}

			inline Address & operator = (const std::string &value) {
				return set(value);
			}

			inline Address & operator = (const sockaddr_storage & value) {
				return set(value);
			}

			template <typename T>
			inline Address & operator = (const T * value) {
				return set(value);
			}

			std::string to_string() const noexcept;

			std::string nic() const;
			std::string macaddress() const;

		};

		struct Addresses {
			const char *interface_name;		///< @brief Interface name.
			IP::Address address;			///< @brief Interface address.
			IP::Address netmask;			///< @brief Interface netmask.
		};

		/// @brief Enumerate local IP addresses and interfaces.
		UDJAT_API bool for_each(const std::function<bool(const Addresses &addr)> &func);

		/// @brief Get default gateway.
		/// @return The default gateway address.
		UDJAT_API IP::Address gateway();

	}

 }

 namespace std {

	UDJAT_API string to_string(const sockaddr_storage &addr, bool dns = false);

	inline ostream & operator<< (ostream& os, const sockaddr_storage &addr) {
		return os << to_string(addr);
	}

	UDJAT_API string to_string(const sockaddr_in &addr, bool dns = false);

	inline ostream & operator<< (ostream& os, const sockaddr_in &addr) {
		return os << to_string(addr);
	}

	UDJAT_API string to_string(const sockaddr_in6 &addr, bool dns = false);

	inline ostream & operator<< (ostream& os, const sockaddr_in6 &addr) {
		return os << to_string(addr);
	}

	UDJAT_API string to_string(const sockaddr &addr, bool dns = false);

	inline ostream & operator<< (ostream& os, const sockaddr &addr) {
		return os << to_string(addr);
	}

	UDJAT_API string to_string(const in_addr &addr, bool dns = false);

	inline ostream & operator<< (ostream& os, const in_addr &addr) {
		return os << to_string(addr);
	}

 }
