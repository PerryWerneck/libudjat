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
  * @brief Declares URL handler object.
  *
  * References:
  *
  * <https://www.algosome.com/articles/anatomy-of-website-url.html>
  *
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
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

	class UDJAT_API URL::Handler {
	protected:
		Handler() = default;

		struct {
			int code = 0;	///< @brief HTTP status code;
			String message;	///< @brief HTTP status message;
		} status;

	public:

		virtual ~Handler();

		/// @brief Get handler description, usually the URL
		virtual const char * c_str() const noexcept = 0;

		/// @brief Set output header.
		/// @param name The header name.
		/// @param value The header value.
		/// @return This handler.
		virtual Handler & header(const char *name, const char *value);

		/// @brief Get input header.
		/// @param name The header name.
		/// @return The header value, "" if not found.
		virtual const char * header(const char *name) const;

		/// @brief Set requested mime-type.
		/// @param mimetype The mimetype to set.
		/// @return This handler.
		virtual Handler & set(const MimeType mimetype);

		virtual Handler & credentials(const char *user, const char *passwd);

		/// @brief Launch exception on failure code.
		int except(int code, const char *message = "");

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
		/// @retval ECANCELLED Operation was cancelled.		
		virtual int perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const char *data, size_t len)> &progress) = 0;

		/// @brief Test file access (do a 'head' on http[s], check if file exists in file://)
		/// @return Test result.
		/// @retval 200 OK.
		/// @retval 302 OK with redirection.
		/// @retval 401 Unauthorized.
		/// @retval 404 Not found.			
		/// @retval EINVAL Invalid method.
		/// @retval ENODATA Empty URL.
		/// @retval ENOTSUP This handler cannot manage test.
		virtual int test(const HTTP::Method method = HTTP::Head, const char *payload = "");

		/// @brief Get value.
		/// @param Value the response.
		/// @return true if value was updated.
		virtual bool get(Udjat::Value &value, const HTTP::Method method = HTTP::Get, const char *payload = "");

		/// @brief Do a 'get' request.
		/// @param progress progress callback.
		/// @return Server response.
		String get(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total)> &progress);

		String get(const HTTP::Method method = HTTP::Get, const char *payload = "");

		/// @brief Download/update a file with progress.
		/// @param filename The fullpath for the file.
		/// @param writer The secondary writer.
		/// @return true if the file was updated.
		bool get(File::Handler &file, const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total)> &progress);

		bool get(File::Handler &file, const HTTP::Method method = HTTP::Get, const char *payload = "");

		/// @brief Download or update a file with progress, setting the last modified time to the value sent by the host.
		/// @param filename The fullpath for the file.
		/// @param progress The progress callback.
		/// @return true if the file was updated.
		bool get(const char *filename, const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total)> &progress);

		/// @brief Download or update a file with progress, setting the last modified time to the value sent by the host.
		bool get(const char *filename, const HTTP::Method method = HTTP::Get, const char *payload = "");

	};
	
 }

