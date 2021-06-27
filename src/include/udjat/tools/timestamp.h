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

#ifndef TIMESTAMP_H_INCLUDED

	#define TIMESTAMP_H_INCLUDED

	#include <udjat/defs.h>
	#include <ctime>
	#include <string>

	namespace Udjat {

		#define TIMESTAMP_FORMAT_JSON "%Y/%m/%d %H:%M:%S"

		/// @brief A time value (in seconds)
		class UDJAT_API TimeStamp {
		private:
			time_t value;

		public:

			constexpr TimeStamp(time_t t = time(nullptr)) : value(t) { }

			std::string to_string(const char *format = "%x %X") const noexcept;

			inline operator bool() const noexcept {
				return value != 0;
			}

			/// @brief Reseta valor com atraso.
			///
			/// @param seconds	Nº de segundos após o atual para setar.
			TimeStamp & reset(const uint32_t seconds) noexcept {
				value = time(nullptr) + seconds;
				return *this;
			}

			TimeStamp & set(const char *time, const char *format = nullptr);

			TimeStamp & operator=(const time_t t) noexcept {
				value = t;
				return *this;
			}

			TimeStamp & operator=(const char *time) {
				return set(time);
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

			time_t operator-(time_t value) const noexcept {
				return this->value - value;
			}

			time_t operator+(time_t value) const noexcept {
				return this->value + value;
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


#endif // TIMESTAMP_H_INCLUDED
