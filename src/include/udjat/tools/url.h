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
 #include <functional>

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

		/// @brief Do a 'get' request.
		/// @return Server response.
		std::string get() const;

		/// @brief Do a 'post' request.
		/// @param payload Post payload.
		/// @return Server response.
		std::string post(const char *payload) const;

		/// @brief Download/update a file.
		/// @param filename The fullpath for the file.
		/// @return true if the file was updated.
		bool get(const char *filename) const;

		/// @brief Download/update a file with progress callback.
		/// @param filename The fullpath for the file.
		/// @param call progress callback.
		/// @return true if the file was updated.
		bool get(const char *filename, std::function<bool(uint64_t current, uint64_t total)> call) const;

	};

 }

 namespace std {

	UDJAT_API string to_string(const Udjat::URL &url);

	inline ostream& operator<< (ostream& os, const Udjat::URL &url) {
		return os << to_string(url);
	}

 }
