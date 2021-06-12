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

 namespace Udjat {

	/// @brief Abstract value holding untyped data.
	class UDJAT_API Value {
	protected:
		Value() { };

	public:

		Value(const Value &) = delete;
		Value(const Value *) = delete;

		/// @brief Value type.
		enum Type : uint8_t {
			Undefined,			///< @brief 'null' value.
			Signed,				///< @brief Signed integer value.
			Unsigned,			///< @brief Unsigned integer value.
			Real,				///< @brief Double value.
			Percent,			///< @brief Percent value (Float in range 0.00 to 100.0)
			Timestamp,			///< @brief Timestamp value.
			String,				///< @brief UTF-8 string value.
			Boolean,			///< @brief Bool value.
			Array,				///< @brief Array value (ordered list).
			Object				///< @brief Object value (collection of name/value pairs).
		};

		/// @brief Has any value?
		virtual bool isNull() const = 0;

		/// @brief Convert value to string.
		virtual std::string to_string() const = 0;

		/// @brief Convert Value to 'object' and insert child.
		/// @return Null value inserted to object.
		virtual Value & operator[](const char *name) = 0;

		/// @brief Convert Value to 'array' and insert child.
		virtual Value & append(Value &value) = 0;

		/// @brief Value factory.
		virtual Value & getValue(const Type type = Undefined) = 0;

		/// @brief Convert value to 'object' and insert child.
		virtual Value & set(const Value &value) = 0;

		/// @brief Set string to value
		virtual Value & set(const char *value, const Type type = String) = 0;

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

		template <typename T>
		Value & set(const T value) {
			return this->set(std::to_string(value));
		}

		template <typename T>
		Value & operator=(const T value) {
			return set(value);
		}

	};

 }

 template <typename T>
 inline Udjat::Value & operator<<(Udjat::Value &out, T value) {
	return out.set(value);
 }


