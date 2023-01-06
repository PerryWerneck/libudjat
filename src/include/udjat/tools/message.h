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
 #include <string>

 namespace Udjat {

	class UDJAT_API Message : public std::string {
	private:
		size_t index = 0;

	public:
		inline Message(const char *fmt) : std::string{fmt} {
		}

		template<typename... Targs>
		Message(const char *fmt, Targs... Fargs) : std::string{fmt} {
			append(Fargs...);
		}

		Message & append(const char *str);
		Message & append(const bool value);

		inline Message & append(char *str) {
			return append((const char *) str);
		}

		inline Message & append(const std::string &str) {
			return append(str.c_str());
		}

		inline Message & append(std::string &str) {
			return append(str.c_str());
		}

		inline Message & append() {
			return *this;
		}

		template<typename T>
		inline Message & append(const T &str) {
			return append(std::to_string(str).c_str());
		}

		template<typename... Targs>
		Message & append(const char *str, Targs... Fargs) {
			append(str);
			return append(Fargs...);
		}

		template<typename... Targs>
		Message & append(const std::string &str, Targs... Fargs) {
			append(str.c_str());
			return append(Fargs...);
		}

		template<typename... Targs>
		Message & append(const bool value, Targs... Fargs) {
			append(value);
			return append(Fargs...);
		}

		template<typename T, typename... Targs>
		Message & append(const T str, Targs... Fargs) {
			append(str);
			return append(Fargs...);
		}


	};
 }


