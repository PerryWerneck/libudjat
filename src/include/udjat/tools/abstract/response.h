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
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/exception.h>
 #include <functional>
 #include <stdexcept>
 #include <ctime>
 #include <string>

 namespace Udjat {

	namespace Abstract {

		/// @brief Base class for all responses.
		class UDJAT_API Response {
		protected:

			/// @brief Response type.
			MimeType mimetype = MimeType::custom;

			/// @brief Expiration timestamp (For cache headers)
			TimeStamp expiration = 0;

			/// @brief Timestamp of data.
			TimeStamp modification = 0;

			/// @brief Response result.
			struct {

				/// @brief Request error message.
				std::string message;

				/// @brief Extended error message.
				std::string body;

				/// @brief URL for more information.
				std::string url;

				/// @brief Error code.
				int code = 0;

			} status;

		public:

			Response(const MimeType m = MimeType::custom) : mimetype(m) {
			}

			inline operator bool() const noexcept {
				return status.code == 0;
			}

			inline int status_code() const noexcept {
				return status.code;
			}

			inline operator MimeType() const noexcept {
				return this->mimetype;
			}

			inline bool operator ==(const MimeType mimetype) const noexcept {
				return this->mimetype == mimetype;
			}

			inline bool operator !=(const MimeType mimetype) const noexcept {
				return this->mimetype != mimetype;
			}

			Response & failed(int code = errno) noexcept;
			Response & failed(const char *message, const char *body = "", const char *url = "") noexcept;
			Response & failed(int code, const char *message, const char *body = "", const char *url = "") noexcept;

			Response & failed(const std::exception &e) noexcept;

			// Deprecated.
			Response & failed(const std::system_error &e) noexcept;
			Response & failed(const HTTP::Exception &e) noexcept;

			/// @brief Enumerate response properties (http headers).
			virtual void for_each(const std::function<void(const char *property_name, const char *property_value)> &call) const noexcept;

			/// @brief Set item count for this response.
			/// @param value The item count (for X-Total-Count http header).
			virtual void count(size_t value) noexcept;

			/// @brief Set range for this response (Content-Range http header).
			/// @param from First item.
			/// @param to Last item.
			/// @param total Item count.
			virtual void content_range(size_t from, size_t to, size_t total) noexcept;

			/// @brief Convert response to formatted string.
			virtual std::string to_string() const;

			/// @brief Set timestamp for data, ignore zeros.
			/// @return Current value.
			time_t last_modified(const time_t time) noexcept;

			/// @brief Set response expiration time, ignore zeros.
			/// @return Current expiration time.
			time_t expires(const time_t time) noexcept;

			inline time_t last_modified() const noexcept {
				return (time_t) modification;
			}

			inline time_t expires() const noexcept {
				return (time_t) expiration;
			}

			/// @brief Get the message of the first failure on this response.
			inline const char * message() const noexcept {
				return status.message.c_str();
			}

			/// @brief Get the body of the first failure on this response.
			inline const char * body() const noexcept {
				return status.body.c_str();
			}

			/// @brief Get the url for more information about the first failure on this response.
			inline const char * url() const noexcept {
				return status.url.c_str();
			}

		};

	}

 }


