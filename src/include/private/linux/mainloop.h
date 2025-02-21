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
 #include <private/service.h>

namespace Udjat {

	namespace Linux {

		class UDJAT_PRIVATE MainLoop : public Udjat::MainLoop {
		private:

			/// @brief Is the mainloop enabled.
			bool running = true;

			class UDJAT_PRIVATE Timers {
			public:

				/// @brief Minimal timer value.
				unsigned long maxwait = 60000;

				/// @brief List of enabled timers.
				std::list<Timer *> enabled;

			};

			/// @brief Event FD.
			int efd = -1;

			/// @brief Active timers.
			Timers timers;

			/// @brief Active handlers.
			std::list<Handler *> handlers;

			/// @brief Run timers, compute poll timeout.
			/// @return The timeout to next 'poll()' call.
			unsigned long compute_poll_timeout() noexcept;

		protected:

			/// @brief Mutex
			static std::mutex guard;

		public:

			MainLoop();
			virtual ~MainLoop();

			/// @brief Get poll() timeout.
			/// @return poll call timeout.
			inline unsigned long maxwait() const noexcept {
				return timers.maxwait;
			}

			/// @brief Set poll() timeout.
			/// @param value The timeout for poll.
			inline void maxwait(unsigned long value) noexcept {
				timers.maxwait = value;
			}

			/// @brief Run mainloop.
			int run() override;

			bool active() const noexcept override;

			void post(Message *message) noexcept override;

			/// @brief Wakeup main loop.
			void wakeup() noexcept override;

			/// @brief Quit mainloop.
			void quit() override;

			void push_back(MainLoop::Timer *timer) override;
			void remove(MainLoop::Timer *timer) override;

			void push_back(MainLoop::Handler *handler) override;
			void remove(MainLoop::Handler *handler) override;

			bool enabled(const Timer *timer) const noexcept override;
			bool enabled(const Handler *handler) const noexcept override;

			bool for_each(const std::function<bool(Timer &timer)> &func);

		};

	};

}
