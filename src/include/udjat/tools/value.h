/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <vector>
 #include <list>

 namespace Udjat {

	class Report;

	/// @brief Abstract value holding multiple types of data.
	class UDJAT_API Value : public Abstract::Object {
	public:

		/// @brief Value type.
		enum Type : uint8_t {
			Undefined	= '\0',			///< @brief 'null' value.
			Array		= 'a',			///< @brief Array value (ordered list).
			Object		= 'O',			///< @brief Object value (collection of name/value pairs).
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

			Report		= 'R',			///< @brief The value contains a report (internal use only).
		};

	private:

		class Getter;
		friend class Getter;
	
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

	public:

		constexpr Value() : type{Undefined} {
		}
		
		Value(const Value *value) : Value{*value} {
		}
		
		Value(const Value &value);

		Value(Type type);
		
		~Value();

		bool as_bool() const;

		/// @brief Type factory.
		static Type TypeFactory(const XML::Node &node, const char *attrname = "value-type", const char *def = "Undefined");
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

		/// @brief Append item to array.
		/// @return The item.
		Value & append(Value::Type type = Undefined);

		/// @brief Append item to object.
		/// @return The item.
		Value & append(const char *name, Value::Type type = Undefined);

		/// @brief Merge another value.
		Value & merge(const Value &src);

		/// @brief Convert this value to an empty report with defined columns.
		/// @return The report handler.
		Udjat::Report & ReportFactory(const char *column_name, ... ) __attribute__ ((sentinel));

		/// @brief Convert this value to report, add first row.
		/// @param first_row The first row of the report.
		/// @return The report handler.
		Udjat::Report & ReportFactory(const Value &first_row);

		virtual Udjat::Report & ReportFactory(const std::vector<std::string> &column_names);

		/// @brief Get item.
		/// @return The item.
		Value & operator[](int ix);

		/// @brief Get item.
		/// @return The item.
		const Value & operator[](int ix) const;

		bool contains(const char *name) const noexcept;

		/// @brief Get child by name, insert it if not found.
		/// @return Null value inserted to object.
		Value & operator[](const char *name);

		/// @brief Get child by name, exception it if not found.
		/// @return First child with required name. Exception if not found.
		const Value & operator[](const char *name) const;

		/// @brief Get child by type.
		/// @return First child with required type. Exception if not found.
		// const Value & operator[](Type type) const;

		/// @brief Navigate from all values until 'call' returns true.
		/// @return true if 'call' has returned true, false if not.
		bool for_each(const std::function<bool(const char *name, const Value &value)> &call) const;

		/// @brief Navigate from all values until 'call' returns true.
		/// @return true if 'call' has returned true, false if not.
		bool for_each(const std::function<bool(const Value &value)> &call) const;

		/// @brief Clear contents, set value type.
		Value & clear(const Type type = Undefined);

		/// @brief For legacy use only.
		inline Value & reset(const Type type = Undefined) {
			return clear(type);
		}

		/// @brief Set value and type.
		Value & set(const char *value, const Type type = String);

		inline Value & set(char *value, const Type type = String) {
			return set((const char *) value, type);
		}

		inline Value & set(const std::string &value, const Type type = String) {
			return set(value.c_str(),type);
		}

		/// @brief Emit event, allowing modules to change value contents.
		/// @param event_name The event name that will be passed to listeners.
		// void emit_event(const char *event_name, const char *event_data = nullptr);

		Value & set(const Value &value);

		/// @brief Set a percentual from 0.0 to 1.0
		Value & setFraction(const float fraction);
		Value & set(const short value);
		Value & set(const unsigned short value);
		Value & set(const int value);
		Value & set(const unsigned int value);
		Value & set(const TimeStamp &value);
		Value & set(const bool value);
		Value & set(const float value);
		Value & set(const double value);

		/// @brief Load tags <value name='name' value='value' type='type' /> into value.
		Value & set(const XML::Node &node);

		template <typename T>
		Value & set(const T value) {
			return this->set(std::to_string(value));
		}

		template <typename T>
		Value & operator=(const T value) {
			return set(value);
		}

		const Value & get(std::string &value) const;
		const Value & get(short &value) const;
		const Value & get(unsigned short &value) const;
		const Value & get(int &value) const;
		const Value & get(unsigned int &value) const;
		const Value & get(long &value) const;
		const Value & get(unsigned long &value) const;
		const Value & get(TimeStamp &value) const;
		const Value & get(bool &value) const;
		const Value & get(float &value) const;
		const Value & get(double &value) const;

		std::string to_string() const noexcept override;
		std::string to_string(const char *def) const;
		std::string to_string(const MimeType mimetype) const;

		/// @brief Get child value.
		/// @param key The child name.
		/// @param value String to update with the property value.
		/// @return true if the property is valid.
		bool getProperty(const char *key, std::string &value) const override;

		virtual void serialize(std::ostream &out, const MimeType mimetype) const;

		std::string serialize(const MimeType mimetype = MimeType::json) const;

		void to_json(std::ostream &out) const;
		void to_xml(std::ostream &out) const;
		void to_html(std::ostream &out) const;
		void to_yaml(std::ostream &out, size_t left_margin = 0) const;
		void to_sh(std::ostream &stream) const;

		/// @brief Serialize arrays to csv
		void to_csv(std::ostream &out, char delimiter = ',') const;

	};

 };

 namespace std {

	template <typename T>
	inline Udjat::Value & operator<<(Udjat::Value &out, T value) {
		return out.set(value);
	}

	template <typename T>
	inline const Udjat::Value & operator>> (const Udjat::Value &in, T &value ) {
		in.get(value);
		return in;
	}

	UDJAT_API const char * to_string(const Udjat::Value::Type type) noexcept;

	inline string to_string(const Udjat::Value &value) noexcept {
		return value.to_string();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Value &value) {
		return os << value.to_string();
	}

 }




