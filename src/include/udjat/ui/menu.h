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
		class UDJAT_API Menu {
		protected:
			Menu(const char *title);
			std::string title;

		public:
			class UDJAT_API Option {
			public:
				const char *text;
				constexpr Option(const char *t) : text{t} {}
				Option(const std::string &t) : text{t.c_str()} {}

			};

			virtual ~Menu();

			/// @brief Select option, return index or throw system_error(ECANCELLED) if user cancel.
			/// @param options The options to select.
			/// @return The index of the selected option.
			virtual size_t select(const std::vector<const char *> &options) = 0;
			
			size_t select(const Option *options, size_t count);
			size_t select(const Option *options);
			size_t select(const std::vector<Option *> &options);

		};

	}

 }


