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
 #include <cstdint>
 #include <udjat/tools/object.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/timestamp.h>

 namespace Udjat {

	/// @brief Abstract value holding multiple types of data.
	class UDJAT_API Variant : public Abstract::Object {
	public:

		/// @brief Variant types.
		enum Type : uint8_t {
			Undefined	= '\0',			///< @brief 'null' value.
			ObjectPath	= 'p',			///< @brief Object path (string).
			Array		= 'a',			///< @brief Array value (ordered list).
			ValueMap	= 'm',			///< @brief collection of name/value pairs.
			String		= 's',			///< @brief UTF-8 string value.
			Timestamp	= 'T',			///< @brief Timestamp value.
			Signed		= 'S',			///< @brief Signed integer value.
			Unsigned	= 'U',			///< @brief Unsigned integer value.
			Real		= 'd',			///< @brief Double value.
			Boolean		= 'b',			///< @brief Bool value.
			Fraction	= 'F',			///< @brief Fraction value (Float from 0.0 to 1.0).
			Icon		= 'I',			///< @brief Icon name.
			Url			= '@',			///< @brief URL.
			State		= 'A',			///< @brief Level name ('undefined', 'unimportant', 'ready', 'warning', 'error', etc)
			DataTable	= 't',			///< @brief Table with all columns with the same type.

			Object	[[deprecated("Use ValueMap instead.")]] = 'm'
		};

		template <typename T>
		constexpr static Type TypeFactory() {
			return Type::Undefined;
		}

	private:

		class Getter;
		friend class Getter;

		class Table;
		friend class Table;

		Type type = Undefined;

		union Content {
			time_t timestamp;
			int sig;
			unsigned int unsig;
			double dbl;
			void *ptr;

			constexpr Content() : ptr{nullptr} {
			}

		} content;

		Content & append_type(const Type type);
		static void clear(const Type type, Content &content);

	public:

#if __cplusplus >= 201703L	
		constexpr Variant() : type{Undefined} {
		}
#else
		Variant() : type{Undefined} {
		}
#endif
		
		Variant(const Variant *value) : Variant{*value} {
		}
		
		Variant(const Variant &value);

		Variant(Type type);
		
		virtual ~Variant();

		bool as_bool() const;

		/// @brief Type factory.
		static Type TypeFactory(const Udjat::Properties &props, const char *attrname = "value-type", const char *def = "Undefined");
		static Type TypeFactory(const char *name);

		/// @brief Get stored value type.
		inline operator Type() const noexcept {
			return this->type;
		}

		inline bool operator==(const Type type) const noexcept {
			return this->type == type;
		}

		/// @brief Convenience method to compare string values.
		/// @return true if value is string and contents match with str (case insensitive).
		bool operator==(const char *str) const;

		/// @brief Test if value contains a valid string.
		/// @return true if value is a string type and not null.
		bool isString() const noexcept;

		static bool isString(const Type type) noexcept;

		/// @brief Convenient method to get string contents.
		/// @return The contents if value is a string type, empty string if not.
		const char *c_str() const noexcept;

		/// @brief Has any value?
		bool isNull() const noexcept;

		/// @brief Is the value empty?
		bool empty() const noexcept;

		/// @brief Is this a number?
		bool isNumber() const;

		/// @brief Get item count.
		size_t size() const;

		/// @brief Remove item from object.
		Variant & erase(const char *name);

		/// @brief Merge another value.
		Variant & merge(const Variant &src);

		/// @brief Get item.
		/// @return The item.
		Variant & operator[](int ix);

		/// @brief Get item.
		/// @return The item.
		const Variant & operator[](int ix) const;

		bool contains(const char *name) const noexcept;

		/// @brief Get child by name, insert it if not found.
		/// @return Null value inserted to object.
		Variant & operator[](const char *name);

		/// @brief Get child by name, exception it if not found.
		/// @return First child with required name. Exception if not found.
		const Variant & operator[](const char *name) const;

		/// @brief Navigate from all values until 'call' returns true.
		/// @return true if 'call' has returned true, false if not.
		bool for_each(const std::function<bool(const char *name, const Variant &value)> &call) const;

		/// @brief Navigate from all values until 'call' returns true.
		/// @return true if 'call' has returned true, false if not.
		bool for_each(const std::function<bool(const Variant &value)> &call) const;

		/// @brief Clear contents, set value type.
		Variant & clear(const Type type = Undefined);

		inline Variant & operator = (const Type type) {
			clear(type);
			return *this;
		}

		/// @brief For legacy use only.
		inline Variant & reset(const Type type = Undefined) {
			return clear(type);
		}

		Variant & assign(const Variant &value);

		/// @brief Set a percentual from 0.0 to 1.0
		Variant & setFraction(const float fraction);

		/// @brief Set value and type.
		Variant & assign(const char *value, const Type type);

		inline Variant & assign(const std::string &value, const Type type) {
			return assign(value.c_str(),type);
		}

		/// @brief Load tags <value name='name' value='value' type='type' /> into value.
		Variant & assign(const Udjat::Properties &props);

		template <typename T>
		inline Variant & assign(const T value) {
			return this->assign(std::to_string(value),String);
		}

		template <typename T>
		inline Variant & operator=(const T value) {
			return assign(value);
		}

		/// @brief Set value and type.
		Variant & append(const char *value, const Type type);

		inline Variant & append(const std::string &value, const Type type) {
			return append(value.c_str(),type);
		}

		template <typename T>
		inline Variant & append(const T value) {
			return this->append(std::to_string(value),String);
		}

		const Variant & get(std::string &value) const;
		const Variant & get(short &value) const;
		const Variant & get(unsigned short &value) const;
		const Variant & get(int &value) const;
		const Variant & get(unsigned int &value) const;
		const Variant & get(long &value) const;
		const Variant & get(unsigned long &value) const;
		const Variant & get(TimeStamp &value) const;
		const Variant & get(bool &value) const;
		const Variant & get(float &value) const;
		const Variant & get(double &value) const;

		std::string to_string() const noexcept override;
		std::string to_string(const char *def) const;
		std::string to_string(const MimeType mimetype) const;

		Variant & add_column(const char *name, const Type type = String);

		/// @brief Get property value.
		/// @param key The property name.
		/// @param value Object to receive the value.
		/// @return true if the property is valid and value was updated.
		bool get_property(const char *key, Udjat::Variant &value) const override;

		virtual void serialize(std::ostream &out, const MimeType mimetype) const;

		std::string serialize(const MimeType mimetype = MimeType::json) const;

		void to_json(std::ostream &out) const;
		void to_xml(std::ostream &out) const;
		void to_html(std::ostream &out) const;
		void to_yaml(std::ostream &out, size_t left_margin = 0) const;
		void to_sh(std::ostream &stream) const;
		void to_text(std::ostream &stream, size_t left_margin = 0) const;

		/// @brief Serialize arrays to csv
		void to_csv(std::ostream &out, char delimiter = ',') const;

	};

	template <>
	constexpr Variant::Type Variant::TypeFactory<std::string>() {
		return Type::String;
	}

	template <>
	constexpr Variant::Type Variant::TypeFactory<const char *>() {
		return Type::String;
	}

	template <>
	constexpr Variant::Type Variant::TypeFactory<TimeStamp>() {
		return Type::Timestamp;
	}

	template <>
	constexpr Variant::Type Variant::TypeFactory<int>() {
		return Type::Signed;
	}

	template <>
	constexpr Variant::Type Variant::TypeFactory<unsigned int>() {
		return Type::Unsigned;
	}

	template <>
	constexpr Variant::Type Variant::TypeFactory<float>() {
		return Type::Real;
	}

	template <>
	constexpr Variant::Type Variant::TypeFactory<double>() {
		return Type::Real;
	}

	template <>
	constexpr Variant::Type Variant::TypeFactory<bool>() {
		return Type::Boolean;
	}

	template <>
	inline Variant & Variant::assign<const char *>(const char *value) {
		return assign(value,String);
	}

	template <>
	inline Variant & Variant::assign<const std::string &>(const std::string &value) {
		return assign(value.c_str(),String);
	}

	template <>
	inline Variant & Variant::assign<int>(const int value) {
		clear(Signed);
		content.sig = value;
		return *this;
	}

	template <>
	inline Variant & Variant::assign<unsigned int>(const unsigned int value) {
		clear(Unsigned);
		content.unsig = value;
		return *this;
	}

	template <>
	inline Variant & Variant::assign<const TimeStamp &>(const TimeStamp &value) {
		clear(Timestamp);
		content.timestamp = (time_t) value;
		return *this;
	}

	template <>
	inline Variant & Variant::assign<bool>(const bool value) {
		clear(Boolean);
		content.sig = value;
		return *this;
	}

	template <>
	inline Variant & Variant::assign<float>(const float value) {
		clear(Real);
		content.dbl = (double) value;
		return *this;
	}

	template <>
	inline Variant & Variant::assign<double>(const double value) {
		clear(Real);
		content.dbl = value;
		return *this;
	}

	template <>
	inline Variant & Variant::assign(const Abstract::Object &value) {
		clear(ValueMap);
		value.get_properties(*this);
		return *this;
	}

	template <>
	inline Variant & Variant::append<const char *>(const char *value) {
		return append(value,String);
	}

	template <>
	inline Variant & Variant::append<const std::string &>(const std::string &value) {
		return append(value.c_str(),String);
	}

	template <>
	inline Variant & Variant::append<int>(const int value) {
		append_type(Signed).sig = value;
		return *this;
	}

	template <>
	inline Variant & Variant::append<unsigned int>(const unsigned int value) {
		append_type(Unsigned).unsig = value;
		return *this;
	}

	template <>
	inline Variant & Variant::append<const TimeStamp &>(const TimeStamp &value) {
		append_type(Timestamp).timestamp = (time_t) value;
		return *this;
	}

	template <>
	inline Variant & Variant::append<bool>(const bool value) {
		append_type(Boolean).sig = value;
		return *this;
	}

	template <>
	inline Variant & Variant::append<float>(const float value) {
		append_type(Real).dbl = (double) value;
		return *this;
	}

	template <>
	inline Variant & Variant::append<double>(const double value) {
		append_type(Real).dbl = value;
		return *this;
	}

 }

 namespace std {

	template <typename T>
	inline Udjat::Variant & operator<<(Udjat::Variant &out, T value) {
		return out.append(value);
	}

	template <typename T>
	inline const Udjat::Variant & operator>> (const Udjat::Variant &in, T &value ) {
		in.get(value);
		return in;
	}

	UDJAT_API const char * to_string(const Udjat::Variant::Type type);

	inline string to_string(const Udjat::Variant &value) noexcept {
		return value.to_string();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Variant &value) {
		return os << value.to_string();
	}

 }




