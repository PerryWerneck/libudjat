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

 namespace Udjat {

	namespace HTTP {

		/// @brief HTTP exception.
		class UDJAT_API Exception : public Udjat::Exception {
		public:

			/// @brief Error codes.
			struct Codes {
				int http;
				std::error_code system;

				Codes() : http(-1) {
				}

			};

		protected:
			/// @brief HTTP error code.
			unsigned int http_code;

		public:
			Exception(unsigned int http_code);
			Exception(unsigned int http_code, const char *message);
			Exception(const char *message);

			/// @brief Get http error code.
			inline unsigned int code() const noexcept {
				return http_code;
			}

			/// @brief Translate http error to system error.
			/// @param http_code http error code.
			/// @return The corresponding system error code (or -1 if there's no one).
			static int syscode(unsigned int http_code) noexcept;

			/// @brief Translate system error to http.
			/// @param syscode system error code.
			/// @return The corresponding http error code (or 500 if there's no one).
			static int code(int syscode) noexcept;

			/// @brief Translate system error to http.
			/// @param except system error.
			/// @return The corresponding http error code (or 500 if there's no one).
			static int code(const std::system_error &except) noexcept;

		};
	}

 }

