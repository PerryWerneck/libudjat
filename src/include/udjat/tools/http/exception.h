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
 #include <system_error>
 #include <string>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/http/status.h>

 namespace Udjat {

	namespace HTTP {

		/// @brief HTTP exception.
		class UDJAT_API Exception : public std::runtime_error, public HTTP::Status {
		public:
			Exception(StatusCode code);
			Exception(StatusCode code, const char *message);
			Exception(const char *message);

			/// @brief Get http error code.
			inline unsigned int code() const noexcept {
				return Status::code;
			}

			// /// @brief Translate system error to http.
			// /// @param syscode system error code.
			// /// @return The corresponding http error code (or 500 if there's no one).
			// static int code(int syscode) noexcept;

			// /// @brief Translate system error to http.
			// /// @param except system error.
			// /// @return The corresponding http error code (or 500 if there's no one).
			// static int code(const std::system_error &except) noexcept;

		};
	}

 }

