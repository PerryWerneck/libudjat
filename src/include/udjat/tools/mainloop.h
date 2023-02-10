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

#ifdef _WIN32
	#include <winsock2.h>	// WSAPOLLFD
	#include <windows.h>
#else
	#include <poll.h>
#endif // _WIN32

#include <udjat/defs.h>
#include <udjat/moduleinfo.h>

#include <list>
#include <functional>
#include <mutex>
#include <ostream>
#include <udjat/request.h>

namespace Udjat {

	class UDJAT_API MainLoop {
	public:

		class Timer;
		class Handler;
		class Service;

	protected:

		class Timers;

		MainLoop() {}

		/// @brief Mutex
		static std::mutex guard;

		/// @brief Is the mainloop enabled.
		bool running = true;

		//
		// File/socket/Handle management
		//
		std::list<Handler *> handlers;

	public:

		MainLoop(const MainLoop &src) = delete;
		MainLoop(const MainLoop *src) = delete;

		virtual ~MainLoop();

		/// @brief Get default mainloop.
		static MainLoop & getInstance();

		virtual void push_back(MainLoop::Service *service) = 0;
		virtual void remove(MainLoop::Service *service) = 0;

		/// @brief Is timer enabled?
		virtual bool enabled(const Timer *timer) const noexcept = 0;

		virtual void push_back(MainLoop::Timer *timer) = 0;
		virtual void remove(MainLoop::Timer *timer) = 0;

		/// @brief Run mainloop.
		virtual int run() = 0;

		/// @brief Is the mainloop active?
		inline operator bool() const noexcept {
			return running;
		}

		/// @brief Check if the handler is enabled.
		bool verify(const Handler *handler) const noexcept;

		/// @brief Quit mainloop.
		virtual void quit() = 0;

		/// @brief Wakeup main loop.
		virtual void wakeup() noexcept = 0;

		/// @brief Create timer for callback.
		/// @param interval	Timer interval on milliseconds.
		/// @return Timer object.
		Timer * TimerFactory(unsigned long interval, const std::function<bool()> call);

#ifdef _WIN32
		BOOL post(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

		/// @brief Watch windows object.
		void insert(const void *id, HANDLE handle, const std::function<bool(HANDLE handle,bool abandoned)> call);

		static void insert(HANDLE handle, const std::function<bool(HANDLE handle,bool abandoned)> exec);
		static void remove(HANDLE handle);
#endif // _WIN32

	};

}
