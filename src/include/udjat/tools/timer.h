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

	using Timer=MainLoop::Timer;

	class UDJAT_API MainLoop::Timer {
	private:

		struct {

			/// @brief The timestamp, in getCurrentTime() units, for next activation.
			unsigned long activation_time = 0;

			/// @brief The interval in milliseconds.
			unsigned long interval = 0;

		} values;

	protected:

		/// @brief Timer event.
		virtual void on_timer() = 0;

	public:

		/// @brief Create a disabled timer.
		constexpr Timer() {
		}

		/// @brief The timestamp, in getCurrentTime() units, for next activation.
		inline unsigned long activation_time() const noexcept {
			return values.activation_time;
		}

		inline unsigned long interval() const noexcept {
			return values.interval;
		}

		/// @brief Get Timer value.
		// inline unsigned long value() const noexcept {
		//	return next;
		//}

		/// @brief Activate timer.
		/// @return The timestamp, in getCurrentTime() units, for next activation (or '0' if timer was disabled).
		unsigned long activate() noexcept;

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

		/// @brief Set new interval timer.
		/// @param milliseconds The new timer value.
		/// @return true if the interval was changed.
		bool set(const unsigned long milliseconds);

		inline void reset(const unsigned long milliseconds) {
			set(milliseconds);
		}

		inline Timer & operator = (const unsigned long milliseconds) {
			set(milliseconds);
			return *this;
		}

		/// @brief Get timer as string.
		std::string to_string() const;

		/// @brief Create timer for callback.
		/// @param call Method when timer expires, timer will be deleted if it returns 'false'.
		/// @param interval	Timer interval on milliseconds.
		/// @return Timer object.
		static Timer * Factory(unsigned long interval, const std::function<bool()> call);

	};


 }

