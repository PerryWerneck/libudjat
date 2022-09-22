/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>

 namespace Udjat {

	class UDJAT_API MainLoop::Timer {
	private:

		friend class MainLoop::Timers;

		/// @brief The time of next call.
		unsigned long next = 0;

		/// @brief The interval in milliseconds.
		unsigned long milliseconds = 0;

	protected:

		/// @brief Timer event.
		virtual void on_timer() = 0;

	public:

		/// @brief Create a disabled timer.
		constexpr Timer() {
		}

		/// @brief Is timer enabled?
		bool enabled() const;

		/// @brief Enable timer.
		void enable();

		/// @brief Enable timer.
		/// @param interval Timer value in milliseconds.
		void enable(unsigned long milliseconds);

		/// @brief Disable timer
		void disable();

		/// @brief Create timer.
		/// @param interval Timer value in milliseconds.
		Timer(unsigned long milliseconds);

		virtual ~Timer();

		/// @brief Get current timer.
		static unsigned long getCurrentTime();

		/// @brief Reset timer.
		/// @param milliseconds The new timer value or '0' to activate timer on next cicle.
		void reset(unsigned long milliseconds = 0);

		/// @brief Get timer as string.
		std::string to_string() const;

	};


 }

