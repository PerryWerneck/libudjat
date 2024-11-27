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
 #include <udjat/tools/xml.h>
 #include <ctime>
 #include <string>
 #include <cstdint>
 #include <time.h>

 namespace Udjat {

	#define TIMESTAMP_FORMAT_JSON "%Y/%m/%d %H:%M:%S"

	/// @brief A time value (in seconds)
	class UDJAT_API TimeStamp {
	protected:
		time_t value;

	public:

		constexpr TimeStamp(time_t t = time(nullptr)) : value(t) { }

		explicit TimeStamp(struct ::tm &tm) : value{mktime(&tm)} { }

		explicit TimeStamp(const char *time, const char *format = nullptr);

		TimeStamp(const XML::Node &node, const char *attrname, const char *def = "");
		TimeStamp(const XML::Node &node, const char *attrname, const time_t def);

		std::string to_string(const char *format = "%x %X") const noexcept;

		/// @brief Format as json date/time
		inline std::string to_json() const noexcept {
			return to_string(TIMESTAMP_FORMAT_JSON);
		}

		std::string to_verbose_string() const noexcept;

		inline std::string to_string(const std::string &format) const noexcept {
			return to_string(format.c_str());
		}

		inline operator bool() const noexcept {
			return value != 0;
		}

		operator struct tm() const noexcept;

		/// @brief Set a time delay.
		/// @param seconds	Seconds after current timestamp to set.
		TimeStamp & reset(const uint32_t seconds) noexcept {
			value = ::time(nullptr) + seconds;
			return *this;
		}

		TimeStamp & reset(const char *time, const char *format = nullptr) {
			value = ::time(nullptr) + parse(time,format);
			return *this;
		}

		static time_t parse(const char *time, const char *format = nullptr);

		TimeStamp & set(const char *time, const char *format = nullptr);

		TimeStamp & operator=(const time_t t) noexcept {
			value = t;
			return *this;
		}

		inline TimeStamp & operator=(const char *time) {
			return set(time);
		}

		inline operator time_t() const noexcept {
			return this->value;
		}

		inline time_t as_value() const noexcept {
			return this->value;
		}

		bool operator==(time_t value) const noexcept {
			return this->value == value;
		}

		bool operator<(time_t value) const noexcept {
			return this->value < value;
		}

		bool operator>(time_t value) const noexcept {
			return this->value > value;
		}

		TimeStamp & operator-(time_t value) noexcept {
			this->value -= value;
			return *this;
		}

		time_t operator+(time_t value) noexcept {
			this->value -= value;
			return *this;
		}

		void operator ++() {
			++value;
		}

		void operator --() {
			++value;
		}

		void operator ++ (int) {
			value++;
		}

		void operator -- (int) {
			value--;
		}

	};

 }

 namespace std {

	inline string to_string(const Udjat::TimeStamp &time) {
		return time.to_string();
	}

	inline ostream& operator<< (ostream& os, const Udjat::TimeStamp &time ) {
		return os << time.to_string();
	}

 }


