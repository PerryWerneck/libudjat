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

		///< @brief File/Socket handler
		class UDJAT_API Handler {
		private:
			friend class MainLoop;

			const void *id = nullptr;
			int fd = -1;
			Event events = (Event) 0;
			// time_t running = 0;

		protected:
			virtual bool call(const Event event) const = 0;

		public:
			constexpr Handler(const void *i, int f, const Event e) : id(i), fd(f), events(e) {
			}

			inline bool operator ==(const void *id) const noexcept {
				return id == this->id;
			}

			virtual ~Handler();

		};

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

			/// @brief List services.
			static void getInfo(Response &response);

			virtual void start();
			virtual void stop();

			std::ostream & info() const;
			std::ostream & warning() const;
			std::ostream & error() const;

		};

	protected:
		/// @brief Private constructor, use getInstance() instead.
		MainLoop();

		/// @brief Is the mainloop enabled.
		bool enabled;

	private:

		/// @brief Services
		std::list<Service *> services;

		/// @brief Start services.
		void start() noexcept;

		/// @brief Stop services.
		void stop() noexcept;

		/// @brief Mutex
		static std::mutex guard;

		//
		// Timers.
		//
		class Timer;

		struct Timers {

			/// @brief List of active timers.
			std::list<std::shared_ptr<Timer>> active;

			/// @brief Run timers, return miliseconds to next timer.
			unsigned long run() noexcept;

		} timers;

#ifdef _WIN32
		/// @brief Process windows messages.
		static LRESULT WINAPI hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/// @brief Object window for this loop
		HWND hwnd;

		/// @brief get sockets
		ULONG getHandlers(WSAPOLLFD **fds, ULONG *length);

#else
		/// @brief Event FD.
		int efd;

		/// @brief get FDs.
		nfds_t getHandlers(struct pollfd **fds, nfds_t *length);

#endif // _WIN32

		//
		// File/socket/Handle management
		//
		std::list<std::shared_ptr<Handler>> handlers;

	public:

		MainLoop(const MainLoop &src) = delete;
		MainLoop(const MainLoop *src) = delete;

		~MainLoop();

		/// @brief Get default mainloop.
		static MainLoop & getInstance();

		/// @brief Run mainloop.
		void run();

		/// @brief Is the mainloop active?
		inline operator bool() const noexcept {
			return enabled;
		}

		/// @brief Quit mainloop.
		void quit();

		/// @brief Wakeup main loop.
		void wakeup() noexcept;

		#ifdef _WIN32

			/// @brief Watch windows object.
			void insert(const void *id, HANDLE handle, const std::function<bool()> call);

		#endif // _WIN32

		/// @brief Insert socket/file handler in the list of event sources.
		void push_back(std::shared_ptr<Handler> handler);

		/// @brief Remove socket/file handler.
		void remove(std::shared_ptr<Handler> handler);

		/// @brief Insert socket/file in the list of event sources.
		/// @return Socket/file handler.
		std::shared_ptr<Handler> insert(const void *id, int fd, const Event event, const std::function<bool(const Event event)> call);

		/// @brief Insert timer in the list of event sources.
		/// @param id		Timer id.
		/// @param interval	Timer interval on milliseconds.
		void insert(const void *id, unsigned long interval, const std::function<bool()> call);

		/// @brief Reset timer to new interval.
		/// @param id		Timer id.
		/// @param interval	Timer interval on milliseconds.
		/// @param true if the timer exists.
		bool reset(const void *id, unsigned long interval);

		/// @brief Remove socket/file/timer/module from the list of event sources.
		void remove(const void *id);

#ifdef _WIN32

		BOOL post(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

		static void insert(HANDLE handle, std::function<void(HANDLE handle,bool abandoned)> exec);
		static void remove(HANDLE handle);

#endif // _WIN32

	};

}
