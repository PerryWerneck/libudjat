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

 #pragma once

 #include <udjat/defs.h>
 #include <string.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/abstract/object.h>
 #include <ostream>
 #include <udjat/tools/xml.h>
 #include <functional>
 #include <memory>
 #include <cstdint>

 namespace Udjat {

	/// @brief Abstract value holding multiple types of data.
	class UDJAT_API Value : public Abstract::Object {
	public:

		/// @brief Value type.
		enum Type : uint8_t {
			Undefined,			///< @brief 'null' value.
			Array,				///< @brief Array value (ordered list).
			Object,				///< @brief Object value (collection of name/value pairs).
			String,				///< @brief UTF-8 string value.
			Timestamp,			///< @brief Timestamp value.
			Signed,				///< @brief Signed integer value.
			Unsigned,			///< @brief Unsigned integer value.
			Real,				///< @brief Double value.
			Boolean,			///< @brief Bool value.
			Fraction,			///< @brief Fraction value (Float from 0.0 to 1.0).
			Icon,				///< @brief Icon name.
			Url,				///< @brief URL.
			State,				///< @brief Level name ('undefined', 'unimportant', 'ready', 'warning', 'error, etc)
		};

		/// @brief Get stored value type.
		virtual operator Type() const noexcept;

		inline bool operator==(const Type type) const noexcept {
			return ((Type) *this) == type;
		}

		/// @brief Build simple string value.
		static std::shared_ptr<Value> Factory(const char *str);

		/// @brief Build dummy value.
		static std::shared_ptr<Value> Factory();

		/// @brief Build an object value.
		static std::shared_ptr<Value> ObjectFactory();

		/// @brief Has any value?
		virtual bool isNull() const = 0;

		/// @brief Has any children?
		virtual bool empty() const noexcept;

		/// @brief Is this a number?
		virtual bool isNumber() const;

		/// @brief Get child by name, insert it if not found.
		/// @return Null value inserted to object.
		virtual Value & operator[](const char *name);

		/// @brief Get child by name.
		/// @return First child with required name. Exception if not found.
		const Value & operator[](const char *name) const;

		/// @brief Get child by type.
		/// @return First child with required type. Exception if not found.
		const Value & operator[](Type type) const;

		/// @brief Navigate from all values until 'call' returns true.
		/// @return true if 'call' has returned true, false if not.
		virtual bool for_each(const std::function<bool(const char *name, const Value &value)> &call) const;

		/// @brief Convert Value to 'array' and insert child.
		/// @return Array entry.
		virtual Value & append(const Type type = Object);

		/// @brief Clear contents, set value type.
		virtual Value & reset(const Type type = Undefined) = 0;

		/// @brief Convert value to 'object' and insert child.
		virtual Value & set(const Value &value) = 0;

		/// @brief Set valuse as string and type.
		virtual Value & set(const char *value, const Type type = String);

		/// @brief Emit event, allowing modules to change value contents.
		/// @param event_name The event name that will be passed to listeners.
		void emit_event(const char *event_name, const char *event_data = nullptr);

		/// @brief Set a percentual from 0.0 to 1.0
		virtual Value & setFraction(const float fraction);

		virtual Value & set(const std::string &value, const Type type = String);

		virtual Value & set(const short value);
		virtual Value & set(const unsigned short value);

		virtual Value & set(const int value);
		virtual Value & set(const unsigned int value);

		virtual Value & set(const long value);
		virtual Value & set(const unsigned long value);

		virtual Value & set(const TimeStamp value);
		virtual Value & set(const bool value);

		virtual Value & set(const float value);
		virtual Value & set(const double value);

		/// @brief Load tags <value name='name' value='value' /> into value.
		virtual Value & set(const XML::Node &node);

		template <typename T>
		Value & set(const T value) {
			return this->set(std::to_string(value));
		}

		template <typename T>
		Value & operator=(const T value) {
			return set(value);
		}

		virtual const Value & get(std::string &value) const;

		virtual const Value & get(short &value) const;
		virtual const Value & get(unsigned short &value) const;

		virtual const Value & get(int &value) const;
		virtual const Value & get(unsigned int &value) const;

		virtual const Value & get(long &value) const;
		virtual const Value & get(unsigned long &value) const;

		virtual const Value & get(TimeStamp &value) const;
		virtual const Value & get(bool &value) const;

		virtual const Value & get(float &value) const;
		virtual const Value & get(double &value) const;

		std::string to_string() const noexcept override;
		std::string to_string(const char *def) const;
		std::string to_string(const MimeType mimetype) const;

		/// @brief Get child value.
		/// @param key The child name.
		/// @param value String to update with the property value.
		/// @return true if the property is valid.
		bool getProperty(const char *key, std::string &value) const override;

		virtual void serialize(std::ostream &out, const MimeType mimetype = MimeType::json) const;

		void to_json(std::ostream &out) const;
		void to_xml(std::ostream &out) const;
		void to_html(std::ostream &out) const;
		void to_yaml(std::ostream &out, size_t left_margin = 0) const;
		void to_sh(std::ostream &stream) const;

		/// @brief Serialize arrays to csv
		void to_csv(std::ostream &out, char delimiter = ',') const;

		inline std::string as_string() const {
			return to_string();
		}

		unsigned int as_uint() const;
		int as_int() const;
		bool as_bool() const;

		inline operator bool() const {
			return as_bool();
		}

	};

	template <typename T>
	inline Udjat::Value & operator<<(Udjat::Value &out, T value) {
		return out.set(value);
	}

	template <typename T>
	inline const Udjat::Value & operator>> (const Udjat::Value &in, T &value ) {
		in.get(value);
		return in;
	}

 }

 namespace std {

	UDJAT_API const char * to_string(Udjat::Value::Type type) noexcept;

	inline string to_string(const Udjat::Value &value) noexcept {
		return value.to_string();
	}

	inline ostream & operator<< (ostream& os, const Udjat::Value &value) {
		return os << value.to_string();
	}

 }




