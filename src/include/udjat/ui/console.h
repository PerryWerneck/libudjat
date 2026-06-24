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
 #include <udjat/tools/logger.h>

 namespace Udjat {

	namespace Console {

		/// @brief Write text to console.
		/// @param text The text to write.
		/// @return true if suceeded.
		UDJAT_API bool write(const char *text) noexcept;

		/// @brief The current console stream accepts ANSI decoration?
		/// @return true if the current console stream allows ANSI decoration.
		UDJAT_API bool decorated() noexcept;

		/// @brief Show a status message
		UDJAT_API void status(Logger::Level, const char *domain, const char *message) noexcept;

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

		/// @brief  screen.
		/// @details Write messages to the console with different colors bypassing logger redirection.
		class UDJAT_API Screen : public std::ostream {
		private:
			bool enabled;	///< @brief Store the original Logger::console status.
			
		public:

			/// @brief Build console screen, disable console logger.
			Screen();

			/// @brief Destroy console screen, reenable console logger.
			~Screen();

			Screen & set(const Foreground color);

			/// @brief set bold mode.
			Screen & bold(bool on);

			/// @brief set dim/faint mode.
			Screen & faint(bool on);

			/// @brief set italic mode.
			Screen & italic(bool on);

			/// @brief Show/Hide cursor.
			Screen & cursor(bool on);

			/// @brief Moves cursor up.
			Screen & up(size_t lines = 1);

			/// @brief Moves cursor down.
			Screen & down(size_t lines = 1);

			/// @brief Erase the entire line
			Screen & erase_line();

			static unsigned short width() noexcept;

			/// @brief Show progress bar at cursor line.
			/// @param prefix Small text on the left.
			/// @param url The URL.
			/// @param current Downloaded size.
			/// @param total Total size.
			/// @return Allways false.
			bool progress(const char *prefix, const char *url, uint64_t current, uint64_t total) noexcept;
	
		};

	}

	// Just for the legacy.
	namespace UI {

		using Console = Udjat::Console::Screen;

	}

 }

 namespace std {

	inline Udjat::Console::Screen & operator<< (Udjat::Console::Screen &os, const Udjat::Console::Foreground fg) {
		os.set(fg);
		return os;
	}

 }