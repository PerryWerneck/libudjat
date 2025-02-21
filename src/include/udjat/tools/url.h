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
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/xml.h>
 #include <memory>

 namespace Udjat {

	class UDJAT_API URL : public Udjat::String {
	public:

		class Handler;

		URL() = default;

		URL(const char *str) : Udjat::String{str} {
		}

		URL(const std::string &str) : String{str} {
		}

		URL(const XML::Node &node, const char *attrname = "src", bool required = false) : String{node,attrname,required} {
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

		String scheme() const;
		String hostname() const;
		String servicename() const;
		String path() const;
		String name() const;
		String dirname() const;

		/// @brief Extract mimetype from URL path.
		/// @return The mimetype, 'none' if URL has no extension.
		MimeType mimetype() const;
		
		int port(const char *proto = "tcp") const;

		URL operator + (const char *path);
		URL & operator += (const char *path);

		/// @brief Connect to host
		/// @return A non-blocking, connected socket.
		int connect(unsigned int seconds = 0);

		/// @brief Test if URL refers to a local file (starts with file://, '/' or '.')
		bool local() const;

		/// @brief Iterate over query
		bool for_each(const std::function<bool(const char *name, const char *value)> &func) const;

		String argument(const char *name) const;

		inline String operator[](const char *name) const {
			return argument(name);
		}

		/// @brief Build state object for this URL based on HTTP code.
		/// @param code The http/system error code.
		/// @param msg The error summary.
		/// @return New state based on URL & code.
		std::shared_ptr<Abstract::State> StateFactory(int code, const char *summary = "");

		/// @brief Retrieves a shared pointer to a Handler object for this URL.
		/// @param allow_default A boolean flag indicating whether to allow use of default handler.
		/// @param autoload A boolean flat indicating whether to try load a module using the handler name.
		/// @return std::shared_ptr<Handler> A shared pointer to the Handler object.
		std::shared_ptr<Handler> handler(bool allow_default = true, bool autoload = false) const;

		/// @brief Get value from host.
		/// @param Value the response.
		/// @param method The HTTP method to use.
		/// @param payload The payload to send.
		/// @return true if value was updated.
		bool get(Udjat::Value &value, const HTTP::Method method = HTTP::Get, const char *payload = "") const;

		/// @brief Test file access (do a 'head' on http[s], check if file exists in file://)
		/// @return Test result.
		/// @retval 200 Got response.
		/// @retval 401 Access denied.
		/// @retval 404 Not found.
		/// @retval -EINVAL Invalid method.
		/// @retval -ENODATA Empty URL.
		/// @retval -ENOTSUP No support for test in protocol handler.
		int test(const HTTP::Method method = HTTP::Head, const char *payload = "") const;

		String call(const HTTP::Method method = HTTP::Get, const char *payload = "") const;

		/// @brief Do a 'get' request.
		/// @param progress progress callback.
		/// @return Server response.
		String get(const std::function<bool(uint64_t current, uint64_t total)> &progress) const;

		String post(const char *payload, const std::function<bool(uint64_t current, uint64_t total)> &progress) const;

		String get(const HTTP::Method method = HTTP::Get, const char *payload = "") const;

		inline String post(const char *payload) const {
			return get(HTTP::Post,payload);
		}

		/// @brief Download/update a file with progress.
		/// @param filename The fullpath for the file.
		/// @param writer The secondary writer.
		/// @return true if the file was updated.
		bool get(const char *filename, const std::function<bool(uint64_t current, uint64_t total)> &progress);

		bool get(const char *filename,const HTTP::Method method = HTTP::Get, const char *payload = "");
		
		/// @brief Get URL, save response to cache file.
		/// @param progress The download progress notifier.
		/// @return The cached filename.
		std::string cache(const std::function<bool(double current, double total)> &progress);

		/// @brief Get URL, save response to cache file.
		/// @return The cached filename.
		std::string cache();

	};

 }

 namespace std {

 	inline const char * to_string(const Udjat::URL &url) {
		return url.c_str();
 	}

	inline ostream& operator<< (ostream& os, const Udjat::URL &url) {
		return os << to_string(url);
	}

 }
