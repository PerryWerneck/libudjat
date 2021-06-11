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

	/// @brief Abstract value holding variable data types.
	class UDJAT_API Value {
	public:
		virtual Value & set(const char *value) = 0;
		virtual std::string to_string() const = 0;

		virtual Value & set(const std::string &value);

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

