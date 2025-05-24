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

			virtual Status & title(const char *text);
			virtual Status & sub_title(const char *text);

			virtual Status & show();
			virtual Status & hide();
			
		};

	}

 }


