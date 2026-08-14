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
 #include <udjat/tools/variant.h>
 #include <udjat/tools/http/status.h>
 #include <string>
 #include <ostream>
 #include <ctime>

 namespace Udjat {

	/// @brief Response with status & value
	/// Response for api call in format jsend (https://github.com/omniti-labs/jsend)
	class UDJAT_API Response : public HTTP::Status, public Variant {
	public:
		Response(const MimeType mimetype = MimeType::json) : HTTP::Status{mimetype}, Variant{Variant::ValueMap} {
		}

		virtual ~Response();

		inline bool operator ==(const MimeType mimetype) const noexcept {
			return mimetype == this->mimetype;
		}

		void clear() noexcept override;

		/// @brief Set item count for this response.
		/// @param value The item count (for X-Total-Count http header).
		inline void count(size_t value) noexcept {
			range.count = value;
		}

		/// @brief Set status based on exception.
		/// @param e The exception.
		/// @param body The message details.
		/// @return *this
		inline HTTP::Status & assign(const std::exception &e) noexcept {
			return HTTP::Status::assign(e);
		}

		inline Response & operator=(const std::exception &e) noexcept {
			HTTP::Status::assign(e);
			return *this;
		}

		/// @brief Set contents from HTTP status code.
		/// @param code The status code to set.
		/// @param body The message details.
		/// @return *this
		inline HTTP::Status & assign(HTTP::StatusCode code, const char *body = nullptr) noexcept {
			return HTTP::Status::assign(code,body);
		}

		inline Response & operator=(const HTTP::StatusCode code) noexcept {
			HTTP::Status::assign(code);
			return *this;
		} 

		/// @brief Serialize according to the mimetype.
		/// Uses jsend format (https://github.com/omniti-labs/jsend) for xml, yaml & json.
		/// @param stream Stream to receive the output.
		void serialize(std::ostream &stream) const noexcept override;

		std::string to_string() const noexcept override;

		/// @brief Set range for this response (Content-Range http header).
		/// @param from First item.
		/// @param to Last item.
		/// @param total Item count.
		inline void content_range(size_t from, size_t to, size_t total) noexcept {
			range.from = from;
			range.to = to;
			range.total = total;
		}

		/// @brief Set timestamp for data, ignore zeros.
		/// @return Current value.
		time_t last_modified(const time_t time) noexcept;

		/// @brief Set response expiration time, ignore zeros.
		/// @param timestamp Timestamp of response expiration, should be greater than time(0)
		/// @return Current expiration time.
		time_t expires(const time_t timestamp) noexcept;

	};	

 }

 namespace std {

	inline std::string to_string(const Udjat::Response &response) {
		return response.to_string();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Response &response) {
		response.serialize(os);
		return os;
	}

 }