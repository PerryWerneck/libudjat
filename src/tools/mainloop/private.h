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
 #include <functional>
 #include <csignal>
 #include <unistd.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	class MainLoop::Timer {
	public:

		/// @brief The timer identifier.
		const void *id;

		/// @brief Is the timer running.
		bool running = false;

		/// @brief The interval in milliseconds.
		unsigned long interval;

		/// @brief The interval in milliseconds.
		unsigned long next;

		/// @brief The timer method.
		const function<bool()> call;

		/// @brief Get current timer.
		static unsigned long getCurrentTime();

		/// @brief Create timer.
		Timer(const void *id, unsigned long milliseconds, const function<bool()> call);

		/// @brief Reset timer.
		void reset(unsigned long milliseconds);

	};

	class MainLoop::Handler {
	public:

		const void *id;
		int fd;
		Event events;
		time_t running;			///< @brief Is the callback running?

		const function<bool(const Event event)> call;

		Handler(const void *id, int fd, const Event event, const function<bool(const Event event)> call);

	};

 }
