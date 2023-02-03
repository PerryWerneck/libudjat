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

 #ifdef _WIN32
	#include <ws2ipdef.h>
 #else
	#include <sys/socket.h>
	#include <arpa/inet.h>
 #endif // _WIN32

 namespace Udjat {

	namespace IP {

		class UDJAT_API Address : public sockaddr_storage {
		private:
			static sockaddr_storage StorageFactory(const char *addr);

		public:
			constexpr Address(const sockaddr_storage &a) : sockaddr_storage{a} {
			}

			constexpr Address() : sockaddr_storage{} {
			}

			Address(const char *addr);

			static bool equal(const sockaddr_storage &a, const sockaddr_storage &b);

			inline bool operator==(const sockaddr_storage &storage) const noexcept {
				return equal((sockaddr_storage) *this, storage);
			}

			inline bool operator==(const Address &addr) const noexcept {
				return equal((sockaddr_storage) *this, (sockaddr_storage) addr);
			}

			inline bool empty() const noexcept {
				return ss_family != 0;
			}

			inline operator bool() const noexcept {
				return ss_family != 0;
			}

			inline Address & clear() noexcept {
				ss_family = 0;
				return *this;
			}

			Address & set(const char * value);
			Address & set(const sockaddr_storage & value);

			inline Address & set(const std::string &value) {
				return set(value.c_str());
			}

			inline Address & operator = (const char * value) {
				return set(value);
			}

			inline Address & operator = (const std::string &value) {
				return set(value);
			}

			inline Address & operator = (const sockaddr_storage & value) {
				return set(value);
			}

		};

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

 }
