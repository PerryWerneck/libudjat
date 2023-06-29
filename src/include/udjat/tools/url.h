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
 #include <udjat/tools/string.h>
 #include <string>
 #include <cstring>
 #include <stdexcept>
 #include <system_error>
 #include <functional>
 #include <udjat/tools/method.h>
 #include <pugixml.hpp>

 namespace Udjat {

	class UDJAT_API URL : public Udjat::String {
	public:

		class UDJAT_API Scheme : public std::string {
		public:
			Scheme() : std::string() {
			}

			Scheme(const char *value) : std::string(value) {
			};

			Scheme(const char *value, size_t len) : std::string(value,len) {
			};

			bool operator==(const char *scheme) const noexcept {
				return strcasecmp(c_str(),scheme) == 0;
			}

		};

		/// @brief URL Components.
		struct UDJAT_API Components {
			Scheme scheme;			///< @brief The scheme name.
			std::string hostname;	///< @brief The host name.
			std::string srvcname;	///< @brief The service name or port number.
			std::string path;		///< @brief The request path.
			std::string query;		///< @brief Query data.

			/// @brief Get the port number from srvcname.
			int portnumber() const;

			/// @brief True if the hostname is not empty.
			inline bool remote() const noexcept {
				return !hostname.empty();
			}

		};

		URL() = default;
		URL(const char *str) : Udjat::String{unescape(str)} {
		}

		URL(const std::string &str) : URL{str.c_str()} {
		}

		URL(const pugi::xml_node &node) : URL{node.attribute("src").as_string()} {
		}

		template<typename... Targs>
		URL(const char *str, Targs... Fargs) : String{str} {
			String::append(Fargs...);
		}

		template<typename... Targs>
		URL(const std::string &str, Targs... Fargs) : String{str} {
			String::append(Fargs...);
		}

		template<typename T, typename... Targs>
		URL(const T &str, Targs... Fargs) : String{str} {
			String::append(Fargs...);
		}

		URL operator + (const char *path);
		URL & operator += (const char *path);

		/// @brief Get URL scheme.
		Scheme scheme() const;

		/// @brief Get URL components.
		Components ComponentsFactory() const;

		/// @brief Unescape URL
		static std::string unescape(const char *src);

		/// @brief Unescape.
		URL & unescape();

		/// @brief Test if URL refers to a local file (starts with file://, '/' or '.')
		bool local() const;

		/// @brief Test if URL is relative (starts with '/' or '.')
		inline bool relative() const noexcept {
			return (c_str()[0] == '/' || c_str()[0] == '.');
		}

		/// @brief Test file access (do a 'head' on http[s], check if file exists in file://)
		/// @return Test result.
		/// @retval 200 Got response.
		/// @retval 401 Access denied.
		/// @retval 404 Not found.
		/// @retval EINVAL Invalid method.
		/// @retval ENODATA Empty URL.
		/// @retval ENOTSUP No support for test in protocol handler.
		int test(const HTTP::Method method = HTTP::Head, const char *payload = "") const noexcept;

		/// @brief Do a 'get' request.
		/// @return Server response.
		std::string get() const;

		/// @brief Do a 'get' request.
		/// @param progress progress callback.
		/// @return Server response.
		std::string get(const std::function<bool(uint64_t current, uint64_t total)> &progress) const;

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
		/// @param progress progress callback.
		/// @return true if the file was updated.
		bool get(const char *filename,const std::function<bool(uint64_t current, uint64_t total)> &progress) const;

		/// @brief Get URL, save response to cache file.
		/// @param progress The download progress notifier.
		/// @return The cached filename.
		std::string filename(const std::function<bool(double current, double total)> &progress);

		/// @brief Get URL, save response to cache file.
		/// @return The cached filename.
		std::string filename();

	};

 }

 namespace std {

	UDJAT_API string to_string(const Udjat::URL &url);

	inline ostream& operator<< (ostream& os, const Udjat::URL &url) {
		return os << to_string(url);
	}

 }
