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

		/// @brief Menu Item.
		class Item : public std::string {
			public:
				Item(const char *opt) : std::string{opt} {
				}

				virtual ~Item();

				/// @brief Get string with formatted menu option.
				/// @param optname The option name.
				/// @param decorated true to set string as a decorated console string.
				/// @return The string with the formatted menu option.
				virtual std::string get(const char *name, bool decorated = false) const noexcept;
		};

		/// @brief Simple popup menu with options.
		class UDJAT_API Menu : public std::vector<Item> {
		protected:
			Menu(const char *title);
			std::string title;
			size_t lpp = 26;

		public:


			virtual ~Menu();

			/// @brief Select option, return index or throw system_error(ECANCELLED) if user cancel.
			/// @param options The options to select.
			/// @return The index of the selected option.
			virtual size_t select() = 0;
			
			inline void append(const char *option) {
				this->emplace_back(option);
			}

			void append(Item &item);

			inline void lines_per_page(size_t value) noexcept {
				lpp = value;
			}

			size_t lines_per_page(void) noexcept {
				return lpp;
			}

			void append(const char **options, size_t count);
			void append(const char **options);

		};

	}

 }

 namespace std {

	inline const char * to_string(const Udjat::Dialog::Item &item) {
		return item.c_str();
	}

 }

