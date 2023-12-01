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

 /**
  * @brief Declare extended exception.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>
 #include <string>

 namespace Udjat {

	class UDJAT_API Exception : public std::runtime_error {
	protected:

		/// @brief The exception title.
		std::string title;

		/// @brief The detailed explanation.
		std::string body;

	public:
		/// @brief Create simple exception.
		/// @param message The error message.
		/// @param body The message body.
		Exception(const char *m, const char *b = "");

		/// @brief Exception from system error.
		Exception(int code, const char *b = "");

	};

 }


