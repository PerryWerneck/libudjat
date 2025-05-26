/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Declare application status dialog.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <memory>

 namespace Udjat {

	namespace Dialog {

		/// @brief Status dialog.
		/// @details Abstract status dialog, the implementation depends on the application.
		class UDJAT_API Status {
		private:
			static Status *instance;

		protected:
			Status();
		  
		public:
			Status(const Status &other) = delete;
			Status(Status *other) = delete;
			virtual ~Status();

			static Status & getInstance();

			virtual Status & title(const char *text) noexcept;
			virtual Status & sub_title(const char *text) noexcept;
			virtual Status & icon(const char *icon_name) noexcept;

			/// @brief Set the step in the current operation.
			/// @details This method is used to update the current step in a multi-step operation.
			/// @param current The current step number.
			/// @param total The total number of steps in the operation.
			virtual Status & step(unsigned int current = 0, unsigned int total = 0) noexcept;

			inline Status & operator = (const char *text) noexcept{
				sub_title(text);
				return *this;
			}
			
			virtual Status & show() noexcept;
			virtual Status & hide() noexcept;
			
		};

	}

 }


