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
	class UDJAT_API Response : public Value {
	protected:

		/// @brief The response status.
		HTTP::Status status;

	private:
		const Abstract::Object *object = nullptr;

	public:
		Response(const MimeType m = MimeType::json) {
			status.mimetype = m;
		}

		virtual ~Response();

		inline void set(const Abstract::Object *object) noexcept {
			this->object = object;
		}

		inline HTTP::StatusCode status_code() const noexcept {
			return status.code;
		}

		HTTP::Status & assign(const HTTP::StatusCode code) noexcept;

		inline HTTP::Status & operator=(const HTTP::StatusCode status) noexcept {
			return assign(status);
		} 

		HTTP::Status & failed(const int syscode) noexcept;
		HTTP::Status & failed(const std::exception &e) noexcept;
		HTTP::Status & failed(const char *message, const char *details = nullptr) noexcept;
		HTTP::Status & failed(const char *title,  const char *message, const char *details) noexcept;

		inline HTTP::Status & failed(const std::string &string) noexcept {
			return failed(string.c_str());
		}

		inline const MimeType mimetype() const noexcept {
			return this->status.mimetype;
		}

		inline operator MimeType() const noexcept {
			return this->status.mimetype;
		}

		inline operator HTTP::Status() const noexcept {
			return this->status;
		}
		inline bool operator ==(const MimeType mimetype) const noexcept {
			return this->status.mimetype == mimetype;
		}

		inline bool operator !=(const MimeType mimetype) const noexcept {
			return this->status.mimetype != mimetype;
		}

		inline operator bool() const noexcept {
			return status.code != HTTP::Ok;
		}

		/// @brief Set item count for this response.
		/// @param value The item count (for X-Total-Count http header).
		inline void count(size_t value) noexcept {
			status.range.count = value;
		}

		/// @brief Set response state.
		/// On HTTP responses set the header X-${object_name}-state=${value};${message}
		virtual void state(const char *object_name,const char *value, const char *message);

		inline size_t count() const noexcept {
			return status.range.count;
		}

		/// @brief Set response message.
		inline void message(const char *message) noexcept {
			status.message = message;
		}

		/// @brief Get response message.
		/// @return The response message (Ok if empty).
		const char *message() const noexcept;

		/// @brief Set response title.
		inline void title(const char *title) noexcept {
			status.title = title;
		}

		/// @brief Get response title.
		/// @return The response title.
		inline const char *title() const noexcept {
			return status.title.c_str();
		}

		/// @brief Set response body.
		inline void body(const char *body) noexcept {
			status.body = body;
		}

		/// @brief Get response body.
		/// @return The response details.
		inline const char * body() const noexcept {
			return status.body.c_str();
		}

		/// @brief Set response details.
		inline void details(const char *details) noexcept {
			status.body = details;
		}

		/// @brief Get response details.
		/// @return The response details.
		inline const char * details() const noexcept {
			return status.body.c_str();
		}

		/// @brief Set range for this response (Content-Range http header).
		/// @param from First item.
		/// @param to Last item.
		/// @param total Item count.
		inline void content_range(size_t from, size_t to, size_t total) noexcept {
			status.range.from = from;
			status.range.to = to;
			status.range.total = total;
		}

		/// @brief Serialize according to the mimetype.
		/// Uses jsend format (https://github.com/omniti-labs/jsend) for xml, yaml & json.
		void serialize(std::ostream &stream) const noexcept;

		/// @brief Set 'not-modified' status.
		inline void not_modified() noexcept {
			status.assign(HTTP::NotModified);
		}

		/// @brief Get 'not-modified' status.
		inline bool not_modified() const noexcept {
			return status.code == HTTP::NotModified;
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
			return (time_t) status.last_modified;
		}

		inline time_t expires() const noexcept {
			return (time_t) status.expires;
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