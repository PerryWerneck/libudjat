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
 #include <cstring>
 #include <cerrno>

 namespace Udjat {

	class UDJAT_API Exception : public std::runtime_error {
	protected:

		struct Info {

			/// @brief Error code.
			int code = -1;

			/// @brief The exception title.
			std::string title;

			/// @brief Error explanation.
			std::string body;

			/// @brief URL for more information.
			std::string url;

			Info(int code, const char *title, const char *body, const char *url = "");
			Info(int code, const std::string &title, const std::string &body, const std::string &url = "");

		} info;


	public:

		Exception(int code, const std::string &message, const std::string &body, const std::string &url = "");
		Exception(int code, const char *message, const std::string &body, const std::string &url = "");
		Exception(int code, const char *message, const char *body = "", const char *url = "");

		/// @brief Create simple exception.
		/// @param message The error message.
		/// @param body The message body.
		Exception(const char *message, const char *body = "");

		/// @brief Exception from system error.
		Exception(int code = errno);

		/// @brief The system code.
		inline int syscode() const noexcept {
			return info.code;
		}

		/// @brief The exception title.
		inline const char *title() const noexcept {
			return info.title.c_str();
		}

		/// @brief Error explanation.
		inline const char *body() const noexcept {
			return info.body.c_str();
		}

		/// @brief URL for more information.
		inline const char *url() const noexcept {
			return info.url.c_str();
		}

	};

 }


