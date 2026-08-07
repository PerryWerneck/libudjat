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
 #include <map>

 namespace Udjat {

	/// @brief Object response.
	/// Response for api call in format jsend (https://github.com/omniti-labs/jsend)
	class UDJAT_API Response : public Variant {
	protected:

		/// @brief The response status.
		HTTP::Status http_status;

	private:
		const Abstract::Object *object = nullptr;

	public:
		Response(const MimeType mimetype = MimeType::json) : Variant{Variant::Object}, http_status{mimetype} {
		}

		virtual ~Response();

		inline bool success() const noexcept {
			return http_status.success();
		}

		inline bool failed() const noexcept {
			return http_status.failed();
		}

		inline void set(const Abstract::Object *object) noexcept {
			this->object = object;
		}

		inline HTTP::StatusCode status_code() const noexcept {
			return http_status.code;
		}

		Response & assign(const HTTP::StatusCode code, const char *body = nullptr) noexcept;
		inline Response & operator=(const HTTP::StatusCode status) noexcept {
			return assign(status);
		} 

		Response & assign(const std::exception &e) noexcept;
		inline Response & operator=(const std::exception &e) noexcept {
			return assign(e);
		} 

		// inline Response & assign(const char *body) noexcept {
		// 	return assign(HTTP::SystemError,body);
		// }

		// HTTP::Status & failed(const int syscode) noexcept;
		// HTTP::Status & failed(const std::exception &e) noexcept;
		// HTTP::Status & failed(const char *message, const char *details = nullptr) noexcept;
		// HTTP::Status & failed(const char *title,  const char *message, const char *details) noexcept;

		inline const MimeType mimetype() const noexcept {
			return http_status.mimetype;
		}

		inline operator MimeType() const noexcept {
			return http_status.mimetype;
		}

		inline HTTP::Status & status() noexcept {
			return http_status;
		}

		inline const HTTP::Status & status() const noexcept {
			return http_status;
		}

		inline bool operator ==(const MimeType mimetype) const noexcept {
			return http_status.mimetype == mimetype;
		}

		inline bool operator !=(const MimeType mimetype) const noexcept {
			return http_status.mimetype != mimetype;
		}

		inline operator bool() const noexcept {
			return http_status.code >= 200 && http_status.code <= 299;
		}

		/// @brief Set item count for this response.
		/// @param value The item count (for X-Total-Count http header).
		inline void count(size_t value) noexcept {
			http_status.range.count = value;
		}

		/// @brief Set response state.
		/// On HTTP responses set the header X-${object_name}-state=${value};${message}
		inline void state(const char *object_name, const char *value, const char *message) {
			http_status.state(object_name,value,message);
		}

		inline size_t count() const noexcept {
			return http_status.range.count;
		}

		/// @brief Set response message.
		inline void message(const char *message) noexcept {
			http_status.message = message;
		}

		inline const char * c_str() const noexcept {
			return http_status.c_str();
		}

		/// @brief Get response message.
		/// @return The response message (Ok if empty).
		const char *message() const noexcept;

		/// @brief Set response title.
		inline void title(const char *title) noexcept {
			http_status.title = title;
		}

		/// @brief Get response title.
		/// @return The response title.
		inline const char *title() const noexcept {
			return http_status.title.c_str();
		}

		/// @brief Set response body.
		inline void body(const char *body) noexcept {
			http_status.body = body;
		}

		/// @brief Get response body.
		/// @return The response details.
		inline const char * body() const noexcept {
			return http_status.body.c_str();
		}

		/// @brief Set response details.
		inline void details(const char *details) noexcept {
			http_status.body = details;
		}

		/// @brief Get response details.
		/// @return The response details.
		inline const char * details() const noexcept {
			return http_status.body.c_str();
		}

		/// @brief Set range for this response (Content-Range http header).
		/// @param from First item.
		/// @param to Last item.
		/// @param total Item count.
		inline void content_range(size_t from, size_t to, size_t total) noexcept {
			http_status.range.from = from;
			http_status.range.to = to;
			http_status.range.total = total;
		}

		/// @brief Serialize according to the mimetype.
		/// Uses jsend format (https://github.com/omniti-labs/jsend) for xml, yaml & json.
		void serialize(std::ostream &stream) const;

		/// @brief Set 'not-modified' status.
		inline void not_modified() noexcept {
			http_status.assign(HTTP::NotModified);
		}

		/// @brief Get 'not-modified' status.
		inline bool not_modified() const noexcept {
			return http_status.code == HTTP::NotModified;
		}

		/// @brief Set timestamp for data, ignore zeros.
		/// @return Current value.
		time_t last_modified(const time_t time) noexcept;

		/// @brief Set response expiration time, ignore zeros.
		/// @param timestamp Timestamp of response expiration, should be greater than time(0)
		/// @return Current expiration time.
		time_t expires(const time_t timestamp) noexcept;

		/// @brief Set custom header.
		virtual void header(const char *name, const char *value) noexcept;

		inline time_t last_modified() const noexcept {
			return (time_t) http_status.last_modified;
		}

		inline time_t expires() const noexcept {
			return (time_t) http_status.expires;
		}

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