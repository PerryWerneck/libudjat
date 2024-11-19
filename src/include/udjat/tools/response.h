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
 #include <udjat/tools/value.h>
 #include <string>
 #include <map>

 namespace Udjat {

	/// @brief Object response.
	/// Response for api call in format jsend (https://github.com/omniti-labs/jsend)
	class UDJAT_API Response : public Value {
	public:
		enum State : uint8_t {
			Success = 0,
			Error = 1,
			Failure = 2
		};

	protected:

		class Value : public Udjat::Value {
		private:
			Udjat::Value::Type type = Udjat::Value::Object;
			std::map<std::string,Value> children;
			std::string value;

		public:
			Value();
			virtual ~Value();

			operator Type() const noexcept override;

			bool empty() const noexcept override;
			bool isNull() const override;

			const Udjat::Value & get(std::string &value) const override;

			bool for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const override;
			Udjat::Value & operator[](const char *name) override;

			Udjat::Value & append(const Udjat::Value::Type type = Udjat::Value::Undefined) override;
			Udjat::Value & reset(const Udjat::Value::Type type) override;
			Udjat::Value & set(const char *value, const Type type = String) override;

		};

		Value data;	/// @brief The response data.

		/// @brief Response type.
		MimeType mimetype = MimeType::custom;

		/// @brief Caching information.
		struct {
			/// @brief The expiration time.
			TimeStamp expires = 0;

			/// @brief The last update time.
			TimeStamp last_modified = 0;
		} timestamp;

		struct {
			State value = Success;
			int code = 0;
			bool not_modified = false;
			std::string title;			///< @brief The response title.
			std::string message;		///< @brief The status message.
			std::string details;		///< @brief The status details.
		} status;

		/// @brief Values for content-range & X-Total-Count headers.
		struct {
			size_t from = 0;
			size_t to = 0;
			size_t total = 0;
			size_t count = 0; ///< @brief The item count (for X-Total-Count http header)
		} range;

	public:
		Response(const MimeType m = MimeType::json) : mimetype(m) {
		}

		virtual ~Response();

		Response & failed(const std::exception &e) noexcept;
		Response & failed(const char *message, const char *details = nullptr) noexcept;
		Response & failed(const char *title,  const char *message, const char *details) noexcept;

		inline Response & failed(const std::string &string) noexcept {
			return failed(string.c_str());
		}

		bool isNull() const override;
		Udjat::Value & reset(const Udjat::Value::Type type) override;

		operator Value::Type() const noexcept override;

		inline operator MimeType() const noexcept {
			return this->mimetype;
		}

		inline bool operator ==(const MimeType mimetype) const noexcept {
			return this->mimetype == mimetype;
		}

		inline bool operator !=(const MimeType mimetype) const noexcept {
			return this->mimetype != mimetype;
		}

		inline operator bool() const noexcept {
			return status.code == 0;
		}

		inline int status_code() const noexcept {
			return status.code;
		}

		bool empty() const noexcept override;

		/// @brief Enumerate contents.
		/// @param call Method to call on every 'data' element.
		/// @return true if que enumeration was interrupted by a return 'true' on call.
		bool for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const override;

		/// @brief Get data value by name
		/// @param name Name of the requested value.
		/// @return The data[name] object.
		Udjat::Value & operator[](const char *name) override;

		inline Udjat::Value & contents() {
			return data;
		}

		/// @brief Set item count for this response.
		/// @param value The item count (for X-Total-Count http header).
		inline void count(size_t value) noexcept {
			range.count = value;
		}

		inline size_t count() const noexcept {
			return range.count;
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

		/// @brief Set response details.
		inline void details(const char *details) noexcept {
			status.details = details;
		}

		/// @brief Get response details.
		/// @return The response details.
		inline const char * details() const noexcept {
			return status.details.c_str();
		}

		/// @brief Set range for this response (Content-Range http header).
		/// @param from First item.
		/// @param to Last item.
		/// @param total Item count.
		inline void content_range(size_t from, size_t to, size_t total) noexcept {
			range.from = from;
			range.to = to;
			range.total = total;
		}

		/// @brief Convert response to formatted string.
		/// @see serialize.
		std::string to_string() const noexcept;

		/// @brief Serialize according to the mimetype.
		/// Uses jsend format (https://github.com/omniti-labs/jsend) for xml, yaml & json.
		void serialize(std::ostream &stream) const;

		/// @brief Set 'not-modified' status.
		inline void not_modified(bool state) noexcept {
			status.not_modified = state;
		}

		/// @brief Get 'not-modified' status.
		inline bool not_modified() const noexcept {
			return status.not_modified;
		}

		/// @brief Set timestamp for data, ignore zeros.
		/// @return Current value.
		time_t last_modified(const time_t time) noexcept;

		/// @brief Set response expiration time, ignore zeros.
		/// @param timestamp Timestamp of response expiration, should be greater than time(0)
		/// @return Current expiration time.
		time_t expires(const time_t timestamp) noexcept;

		inline time_t last_modified() const noexcept {
			return (time_t) timestamp.last_modified;
		}

		inline time_t expires() const noexcept {
			return (time_t) timestamp.expires;
		}

	};

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Response::State state);

	inline std::string to_string(const Udjat::Response &response) {
		return response.to_string();
	}

	inline ostream& operator<< (ostream& os, Udjat::Response::State state) {
			return os << to_string(state);
	}

	inline ostream& operator<< (ostream& os, const Udjat::Response &response) {
		response.serialize(os);
		return os;
	}

 }