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
#include <list>
#include <functional>
#include <mutex>

#ifndef _WIN32
	#include <poll.h>
#endif // !_WIN32

namespace Udjat {

	class UDJAT_API MainLoop {
	private:

		class Controller;

		/// @brief Event FD.
		int efd;

		/// @brief Default wait
		time_t wait;

		/// @brief Mutex
		std::mutex guard;

		/// @brief Is the mainloop enabled.
		bool enabled;

		//
		// Timers.
		//
		class Timer;
		std::list<Timer> timers;

		/// @brief Run timers.
		time_t runTimers(time_t wait);

		/// @brief get FDs.
		nfds_t getHandlers(struct pollfd **fds, nfds_t *length);

		//
		// File/socket management
		//

		///< @brief File/Socket handler
		class Handler;
		std::list<Handler> handlers;

	public:
		enum Event : short {
#ifdef _WIN32
			// https://msdn.microsoft.com/en-us/library/windows/desktop/ms740094(v=vs.85).aspx
			oninput         = POLLRDNORM,   		///< @brief There is data to read.
			onoutput        = POLLWRNORM,   		///< @brief Writing is now possible, though a write larger that the available space in a socket or pipe will still block
			onerror         = POLLERR,              ///< @brief Error condition
			onhangup        = POLLHUP,              ///< @brief Hang  up
#else
			oninput         = POLLIN,               ///< @brief There is data to read.
			onoutput        = POLLOUT,              ///< @brief Writing is now possible, though a write larger that the available space in a socket or pipe will still block
			onerror         = POLLERR,              ///< @brief Error condition
			onhangup        = POLLHUP,              ///< @brief Hang  up
#endif // WIN32
		};

		MainLoop();
		~MainLoop();

		/// @brief Get default mainloop.
		static MainLoop & getInstance();

		/// @brief Run mainloop.
		void run();

		/// @brief Quit mainloop.
		void quit();

		/// @brief Wakeup main loop.
		void wakeup() noexcept;

		/// @brief Insert socket/file in the list of event sources.
		void insert(void *id, int fd, const Event event, const std::function<bool(const Event event)> call);

		/// @brief Insert timer in the list of event sources.
		void insert(void *id, time_t seconds, const std::function<bool(const time_t)> call);

		/// @brief Insert and emit a timer.
		void insert(void *id, const std::function<bool(const time_t)> call);

		/// @brief Reset time to 'now'.
		void reset(void *id);

		/// @brief Reset timer to a new value.
		time_t reset(void *id, time_t seconds, time_t value = time(0));

		/// @brief Remove socket/file/timer/module from the list of event sources.
		void remove(void *id);

	};

}
