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

 namespace Udjat {

	namespace UI {

		/// @brief Console writer.
		/// @details Write messages to the console with different colors bypassing logger redirection.
		class UDJAT_API Console : public std::ostream {
		private:
			bool enabled;
			
		public:

			enum Foreground : uint8_t {
				Default = 39,
				Black = 30,
				Red = 31,
				Green = 32,
				Yellow = 33,
				Blue = 34,
				Magenta = 35,
				Cyan = 36,
				White = 37
			};

			Console();
			~Console();

			Console & set(const Foreground color);

			/// @brief Console writer.
			/// @param message The message to write.
			/// @param color The color of the message.
			/// @param icon The icon to use.
			/// void write(const std::string &message, const std::string &color, const std::string &icon) {
			//	std::cout << "\033[" << color << "m" << icon << " " << message << "\033[0m" << std::endl;
			//}

			/// @brief set bold mode.
			void bold(bool on);

			/// @brief set dim/faint mode.
			void faint(bool on);

			/// @brief set italic mode.
			void italic(bool on);

			unsigned short width() const noexcept;

			bool progress(const char *prefix, const char *url, uint64_t current, uint64_t total) noexcept;

			
		};

	}


 }

 namespace std {

	inline Udjat::UI::Console & operator<< (Udjat::UI::Console &os, const Udjat::UI::Console::Foreground fg) {
		os.set(fg);
		return os;
	}

 }