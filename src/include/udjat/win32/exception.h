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
 #include <string>
 #include <stdexcept>
 #include <udjat/win32/exception.h>

 namespace Udjat {

	namespace Win32 {

		/// @brief Win32 standard error
		class UDJAT_API Exception : public std::runtime_error {
		public:
			Exception(const std::string & what_arg, const DWORD error = GetLastError()) : runtime_error(format(what_arg.c_str(),error)) {
			}

			Exception(const char * what_arg, const DWORD error = GetLastError()) : runtime_error(format(what_arg,error)) {
			}

			static std::string format(const char *what_arg, const DWORD error = GetLastError()) noexcept;
			static std::string format(const DWORD error = GetLastError()) noexcept;

		};

		/// @brief Win32 WSA error
		class UDJAT_API WSAException : public std::runtime_error {
		public:
			WSAException(const std::string & what_arg, const DWORD error = WSAGetLastError()) : runtime_error(format(what_arg.c_str(),error)) {
			}

			WSAException(const char * what_arg, const DWORD error = WSAGetLastError()) : runtime_error(format(what_arg,error)) {
			}

			static std::string format(const char *what_arg, const DWORD error = WSAGetLastError()) noexcept;
			static std::string format(const DWORD error = WSAGetLastError()) noexcept;

		};

	}
 }
