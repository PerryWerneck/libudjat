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
 #include <udjat/tools/xml.h>
 #include <memory>

 namespace Udjat {

	class UDJAT_API URL : public Udjat::String {
	public:

		class UDJAT_API Handler {
		private:
			const URL &url;
	
		protected:
			Handler(const URL &url);

			/// @brief Set connected socket.
			void socket(int sock);

		public:

			virtual ~Handler();

			/// @brief Launch exception on failure code.
			int except(int code, const char *message = "");

			/// @brief Set output header.
			/// @param name The header name.
			/// @param value The header value.
			virtual void header(const char *name, const char *value);

			/// @brief Get input header.
			/// @param name The header name.
			/// @return The header value, "" if not found.
			virtual const char * header(const char *name);

			virtual Handler & set(const MimeType mimetype);

			/// @brief Get header sent by host.
			/// @param name The header name
			/// @return The header value.
			virtual String response(const char *name) const;

			inline const char * c_str() const {
				return url.c_str();
			}

			class Factory {
			private:
				const char *name;

			public:
				Factory(const char *n);
				virtual ~Factory();

				inline bool operator ==(const char *n) const noexcept {
					return strcasecmp(n,name) == 0;
				}

				virtual std::shared_ptr<Handler> HandlerFactory(const URL &url) const = 0;

			};

			/// @brief Perform request.
			/// @param method The HTTP method to use.
			/// @param payload The payload to send.
			/// @param progress The progress callback.
			/// @return HTTP return code.
			/// @retval 200 OK.
			/// @retval 401 Unauthorized.
			/// @retval 404 Not found.			
			virtual int perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const char *data, size_t len)> &progress) = 0;

			/// @brief Test file access (do a 'head' on http[s], check if file exists in file://)
			/// @return Test result.
			/// @retval 200 OK.
			/// @retval 401 Unauthorized.
			/// @retval 404 Not found.			
			/// @retval EINVAL Invalid method.
			/// @retval ENODATA Empty URL.
			/// @retval ENOTSUP This handler cannot manage test.
			virtual int test(const HTTP::Method method = HTTP::Head, const char *payload = "");

			/// @brief Get value.
			/// @param Value the response.
			/// @return true if value was updated.
			virtual bool get(Udjat::Value &value);

			/// @brief Do a 'get' request.
			/// @param progress progress callback.
			/// @return Server response.
			virtual String get(const std::function<bool(uint64_t current, uint64_t total)> &progress);

			/// @brief Download/update a file with progress.
			/// @param filename The fullpath for the file.
			/// @param writer The secondary writer.
			/// @return true if the file was updated.
			virtual bool get(const char *filename, const std::function<bool(uint64_t current, uint64_t total)> &progress);

			String get();

			bool get(const char *filename);

		};

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

		const String scheme() const;
		const String hostname() const;
		const String servicename() const;
		const String path() const;

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

		std::shared_ptr<Handler> handler() const;

		/// @brief Get value.
		/// @param Value the response.
		/// @return true if value was updated.
		inline bool get(Udjat::Value &value) const {
			return handler()->get(value);
		}

		/// @brief Test file access (do a 'head' on http[s], check if file exists in file://)
		/// @return Test result.
		/// @retval 200 Got response.
		/// @retval 401 Access denied.
		/// @retval 404 Not found.
		/// @retval EINVAL Invalid method.
		/// @retval ENODATA Empty URL.
		/// @retval ENOTSUP No support for test in protocol handler.
		inline int test(const HTTP::Method method = HTTP::Head, const char *payload = "") const {
			return handler()->test(method,payload);
		}

		String call(const HTTP::Method method = HTTP::Get, const char *payload = "") const;

		/// @brief Do a 'get' request.
		/// @param progress progress callback.
		/// @return Server response.
		inline String get(const std::function<bool(uint64_t current, uint64_t total)> &progress) const {
			return handler()->get(progress);
		}

		inline String get() const {
			return handler()->get();
		}

		/// @brief Download/update a file with progress.
		/// @param filename The fullpath for the file.
		/// @param writer The secondary writer.
		/// @return true if the file was updated.
		inline bool get(const char *filename, const std::function<bool(uint64_t current, uint64_t total)> &progress) {
			return handler()->get(filename,progress);
		}

		inline bool get(const char *filename) {
			return handler()->get(filename);
		}

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
