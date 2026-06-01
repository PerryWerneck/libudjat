/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <string>
 #include <vector>

 namespace Udjat {

	namespace Dialog {

		/// @brief Simple popup menu with options.
		class UDJAT_API Menu : public std::vector<std::string> {
		protected:
			Menu(const char *title);
			std::string title;

		public:

			virtual ~Menu();

			/// @brief Select option, return index or throw system_error(ECANCELLED) if user cancel.
			/// @param options The options to select.
			/// @return The index of the selected option.
			virtual size_t select() = 0;
			
			inline void append(const char *option) {
				this->emplace_back(option);
			}
			
			void append(const char **options, size_t count);
			void append(const char **options);

		};

	}

 }


