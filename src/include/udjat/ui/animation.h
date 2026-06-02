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
  * @brief Declare Console Writer.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <string>
 #include <cstdint>
 #include <ostream>
 #include <memory>

 namespace Udjat {

	namespace UI {

		class UDJAT_API Animation {
		protected:
			size_t current = 0;
			constexpr Animation() = default;

		public:
			/// The animation styles
			enum Style : uint8_t {
				PlainText,
				Simple,
				Braille,
				Circle,

				Default
			};

			/// @brief  Set default animation style.
			static void set(Animation::Style style = Animation::Style::Default);

			static std::shared_ptr<Animation> Factory(Animation::Style style = Animation::Style::Default);

			virtual const char * get() noexcept = 0;

			inline operator const char *() noexcept {
				return get();
			}

			inline const char * c_str() noexcept {
				return get();
			}

			inline operator int() const noexcept {
				return current;
			}

		private:
			static Style style;

		};


	}

 }

 namespace std {

	inline ostream & operator<< (ostream& os, Udjat::UI::Animation &animation) {
		return os << animation.get();
	}

 }