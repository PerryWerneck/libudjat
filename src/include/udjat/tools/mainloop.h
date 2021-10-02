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
#include <udjat.h>

#ifndef _WIN32
	#include <poll.h>
#endif // !_WIN32

namespace Udjat {

	class UDJAT_API MainLoop {
	public:

		/// @brief Service who can be started/stopped.
		class Service {
		private:
			friend class MainLoop;

			/// @brief Mainloop control semaphore
			static std::mutex guard;

			/// @brief Is the service active?
			bool active = false;

		protected:
			const ModuleInfo *info;

		public:
			Service(const ModuleInfo *info);
			Service();
			virtual ~Service();

			inline bool isActive() const noexcept {
				return active;
			}

			virtual void start();
			virtual void stop();

		};

	private:

		/// @brief Services
		std::list<Service *> services;

		/// @brief Event FD.
		int efd;

		/// @brief Mutex
		static std::mutex guard;

		/// @brief Is the mainloop enabled.
		bool enabled;

		//
		// Timers.
		//
		class Timer;

		struct Timers {

			/// @brief List of active timers.
			std::list<Timer> active;

			/// @brief Run timers, return miliseconds to next timer.
			unsigned long run() noexcept;

		} timers;

#ifndef _WIN32
		/// @brief get FDs.
		nfds_t getHandlers(struct pollfd **fds, nfds_t *length);
#endif // _WIN32

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

		MainLoop(const MainLoop &src) = delete;
		MainLoop(const MainLoop *src) = delete;

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
		void insert(const void *id, int fd, const Event event, const std::function<bool(const Event event)> call);

		/// @brief Insert timer in the list of event sources.
		/// @param id		Timer id.
		/// @param interval	Timer interval on miliseconds.
		void insert(const void *id, unsigned long interval, const std::function<bool()> call);

		/// @brief Reset timer to new interval.
		/// @param id		Timer id.
		/// @param interval	Timer interval on miliseconds.
		void reset(const void *id, unsigned long interval);

		/// @brief Remove socket/file/timer/module from the list of event sources.
		void remove(const void *id);

	};

}
