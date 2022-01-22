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

 /**
  * @brief Declares URL object.
  *
  * References:
  *
  * <https://www.algosome.com/articles/anatomy-of-website-url.html>
  *
  */

 #pragma once

 #include <udjat/defs.h>
 #include <string>
 #include <stdexcept>
 #include <system_error>

 namespace Udjat {

	class UDJAT_API URL : public std::string {
	public:

		/// @brief URL Components.
		struct Components {
			std::string scheme;		///< @brief The scheme name.
			std::string hostname;	///< @brief The host name.
			std::string srvcname;	///< @brief The service name or port number.
			std::string path;		///< @brief The request path.
			std::string query;		///< @brief Query data.

			/// @brief Get the port number from srvcname.
			int portnumber() const;

		};

		URL() = default;
		URL(const char *str) : std::string(unescape(str)) {
		}

		URL(const std::string &str) : URL(str.c_str()) {
		}

		/// @brief Get URL scheme.
		std::string scheme() const;

		/// @brief Get URL components.
		Components ComponentsFactory() const;

		/// @brief Unescape URL
		static std::string unescape(const char *src);

		/// @brief Get URL.
		const char *c_str() const noexcept;

		/// @brief Do a 'get' request.
		std::string get() const;

		/// @brief Do a 'post' request.
		std::string post(const char *payload) const;

	};

 }

 namespace std {

	inline string to_string(const Udjat::URL &url) {
		return url.c_str();
	}

	inline ostream& operator<< (ostream& os, const Udjat::URL &url) {
		return os << url.c_str();
	}

 }
