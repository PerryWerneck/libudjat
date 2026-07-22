/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/http/statuscodes.h>
 #include <udjat/tools/http/mimetype.h>
 #include <string>
 #include <functional>

 namespace Udjat {

	namespace HTTP {

		class UDJAT_API Status {
		public:

			StatusCode code = HTTP::Ok;

			/// @brief The response title.
			std::string title;			

			/// @brief The status message.
			std::string message;		

			/// @brief Mimetype for responses.
			MimeType mimetype = MimeType::none;

			/// @brief The body for status dialog.
			std::string body;

			std::string domain;
			std::string url;
			std::string category;

			/// @brief The expiration time (0 to disable caching).
			time_t expires = (time_t) -1;

			/// @brief The last update time (for caching information).
			time_t last_modified = 0;

			/// @brief Values for content-range & X-Total-Count headers.
			struct {

				/// @param from First item.
				size_t from = 0;
				
				/// @param to Last item.
				size_t to = 0;

				/// @param total Item count.
				size_t total = 0;

				/// @brief The item count (for X-Total-Count http header)
				size_t count = 0; 

			} range;

			/// @brief Build empty status.
			Status(StatusCode c = Ok, const char *message = nullptr);

			Status(StatusCode code, const MimeType mimetype);

			Status(const MimeType mimetype) : Status{Ok, mimetype} {				
			}

			/// @brief Build status from exception.
			/// @param e The exception for status.
			Status(const std::exception &e, const MimeType mimetype = MimeType::none);

			inline operator bool() const noexcept {
				return code != HTTP::Ok;
			}

			Status & clear() noexcept;
			Status & assign(const std::exception &e) noexcept;

			/// @brief Set contents from HTTP status code.
			/// @param code The status code to set.
			/// @return *this
			Status & assign(HTTP::StatusCode code, const char *message = nullptr) noexcept;

			inline Status & operator=(const HTTP::StatusCode code) noexcept {
				return assign(code);
			} 

			inline Status & operator=(const MimeType mimetype) noexcept {
				this->mimetype = mimetype;
				return *this;
			} 

			/// @brief Set contents from syscode.
			/// @param syscode System code to set (From errno).
			/// @return *this;
			Status & assign(int syscode, const char *message = nullptr);

			inline Status & operator=(const int syscode) noexcept {
				return assign(syscode);
			} 

			inline Status & operator=(const std::exception &e) noexcept {
				return assign(e);
			} 

			inline bool success() const noexcept {
				return code == HTTP::Ok || code == HTTP::NoContent;
			}

			/// @brief Serialize according to the mimetype.
			/// Uses jsend format (https://github.com/omniti-labs/jsend) for xml, yaml & json.
			/// @param mimetype The requested mimetype.
			/// @param stream Stream to serialize.
			void serialize(std::ostream &stream) const noexcept;

			std::string to_string() const;
			
			Status & failed(int syscode) noexcept;
			Status & failed(const std::exception &e) noexcept;
			Status & failed(const char *message, const char *details = nullptr) noexcept;
			Status & failed(const char *title,  const char *message, const char *details) noexcept;

			/// @brief Translate http error to system error.
			/// @param http_code http error code.
			/// @return The corresponding system error code (or -1 if there's no one).
			static int syscode(const StatusCode code) noexcept;

			/// @brief Build http headers.
			/// @param callback Callback method to receive the headers.
			void http_headers(const std::function<void(const char *name, const char *value)> &callback) const noexcept;

		};


	}

 }

 namespace std {

	inline const char * to_string(const Udjat::HTTP::Status &status) {
		return status.message.c_str();
	}

	inline ostream& operator<< (ostream& os, const Udjat::HTTP::Status &status) {
		return os << status.message;
	}

}