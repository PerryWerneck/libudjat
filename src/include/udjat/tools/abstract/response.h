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
 #include <stdexcept>
 #include <ctime>

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

				/// @brief Is the request suceeded?
				bool success = true;

				/// @brief Request error message.
				std::string message;

				/// @brief Error code.
				int code = 0;

			} status;

		public:

			Response(const MimeType m = MimeType::custom) : mimetype(m) {
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

			void failed(int code = errno) noexcept;
			void failed(const char *message, int code = 0) noexcept;
			void failed(const std::system_error &e) noexcept;
			void failed(const std::exception &e) noexcept;
			virtual void failed(const HTTP::Exception &e) noexcept;

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

		};

	}

 }


