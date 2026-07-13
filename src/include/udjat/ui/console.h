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
		UDJAT_API void status(Logger::Level level, int column, const char *title, const char *subtitle = nullptr) noexcept;
		
		inline void status(Logger::Level level, const char *title, const char *subtitle = nullptr) noexcept {
			status(level,0,title,subtitle);
		}

		/// @brief Get icon based on logger level
		UDJAT_API const char * icon(Logger::Level level);

		/// @brief Show a success message.
		UDJAT_API bool success(const char *message);

		/// @brief Show a failed message.
		UDJAT_API bool failed(const char *message);

		// Reference: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

		using Function = const char *;
		constexpr Function Reset = "\x1B[0m";
		constexpr Function CursorInvisible = "\x1b[?25l";
		constexpr Function CursorVisible = "\x1b[?25h";
		constexpr Function ClearEOL = "\x1b[0K";
		constexpr Function EraseLine = "\x1b[2K";

		using Decorator = const char *;
		constexpr Decorator SetBold = "\x1B[1m";
		constexpr Decorator SetFaint = "\x1B[2m";
		constexpr Decorator SetItalic = "\x1B[3m";
		constexpr Decorator SetUnderline = "\x1B[4m";
		constexpr Decorator SetBlinking = "\x1B[5m";
		constexpr Decorator SetStrikethrough = "\x1B[9m";

		constexpr Decorator ResetBold = "\x1B[22m";
		constexpr Decorator ResetFaint = "\x1B[22m";
		constexpr Decorator ResetItalic = "\x1B[23m";
		constexpr Decorator ResetUnderline = "\x1B[24m";
		constexpr Decorator ResetBlinking = "\x1B[55m";
		constexpr Decorator ResetStrikethrough = "\x1B[29m";

		using Color = const char *;
		constexpr Color DefaultForeground = "\x1B[39m";
		constexpr Color BlackForeground = "\x1B[30m";
	
		constexpr Color RedForeground = "\x1B[31m";
		constexpr Color GreenForeground = "\x1B[32m";
		constexpr Color YellowForeground = "\x1B[33m";
		constexpr Color BlueForeground = "\x1B[34m";
		constexpr Color MagentaForeground = "\x1B[35m";
		constexpr Color CyanForeground = "\x1B[36m";
		constexpr Color WhiteForeground = "\x1B[37m";

		constexpr Color BrightRedForeground = "\x1B[91m";
		constexpr Color BrightGreenForeground = "\x1B[92m";
		constexpr Color BrightYellowForeground = "\x1B[93m";
		constexpr Color BrightBlueForeground = "\x1B[94m";
		constexpr Color BrightMagentaForeground = "\x1B[95m";
		constexpr Color BrightCyanForeground = "\x1B[96m";
		constexpr Color BrightWhiteForeground = "\x1B[97m";

		/// @brief Get color based on logger level
		UDJAT_API const Color color(Logger::Level level);

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
			[[deprecated("Use Console::Progress")]] bool progress(const char *prefix, const char *url, uint64_t current, uint64_t total) noexcept;
	
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