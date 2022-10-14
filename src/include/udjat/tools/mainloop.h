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
#include <udjat/moduleinfo.h>

#ifndef _WIN32
	#include <poll.h>
#endif // _WIN32

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

		/// @brief Service who can be started/stopped.
		class UDJAT_API Service {
		private:
			friend class MainLoop;

			/// @brief Mainloop control semaphore
			static std::mutex guard;

			/// @brief Service state.
			struct {
				/// @brief Is the service active?
				bool active = false;
			} state;

			/// @brief Service module.
			const ModuleInfo &module;

		protected:
			/// @brief Service name.
			const char *service_name = "service";

		public:
			Service(const Service &src) = delete;
			Service(const Service *src) = delete;

			Service(const char *name, const ModuleInfo &module);
			Service(const ModuleInfo &module);
			virtual ~Service();

			const char * name() const noexcept {
				return service_name;
			}

			inline const char * description() const noexcept {
				return module.description;
			}

			inline const char * version() const noexcept {
				return module.version;
			}

			inline bool isActive() const noexcept {
				return state.active;
			}

			inline bool active() const noexcept {
				return state.active;
			}

			/// @brief List services.
			static void getInfo(Response &response);

			virtual void start();
			virtual void stop();

			std::ostream & info() const;
			std::ostream & warning() const;
			std::ostream & error() const;

		};

	private:

		/// @brief Private constructor, use getInstance() instead.
		MainLoop();

		/// @brief Services
		std::list<Service *> services;

		/// @brief Start services.
		void start() noexcept;

		/// @brief Stop services.
		void stop() noexcept;

		/// @brief Mutex
		static std::mutex guard;

		//
		// Timer controller
		//
		struct Timers {

			/// @brief List of enabled timers.
			std::list<Timer *> enabled;

			/// @brief Run timers, return miliseconds to next timer.
			unsigned long run() noexcept;

		} timers;

		/// @brief Is the mainloop enabled.
		bool enabled = true;

#ifdef _WIN32
		/// @brief Process windows messages.
		static LRESULT WINAPI hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/// @brief Object window for this loop
		HWND hwnd = 0;

		/// @brief get sockets
		ULONG getHandlers(WSAPOLLFD **fds, ULONG *length);

#else
		/// @brief Event FD.
		int efd = -1;

#endif // _WIN32

		//
		// File/socket/Handle management
		//
		std::list<Handler *> handlers;

	public:

		MainLoop(const MainLoop &src) = delete;
		MainLoop(const MainLoop *src) = delete;

		~MainLoop();

		/// @brief Get default mainloop.
		static MainLoop & getInstance();

		/// @brief Run mainloop.
		int run();

		/// @brief Is the mainloop active?
		inline operator bool() const noexcept {
			return enabled;
		}

		/// @brief Check if the handler is enabled.
		bool verify(const Handler *handler) const noexcept;

		/// @brief Quit mainloop.
		void quit();

		/// @brief Wakeup main loop.
		void wakeup() noexcept;

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
