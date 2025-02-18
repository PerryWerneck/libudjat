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

 #pragma once

 #include <config.h>
 #include <winsock2.h>
 #include <windows.h>

 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>
 #include <list>

 #define IDT_CHECK_TIMERS	1

 namespace Udjat {

	namespace Win32 {

		class UDJAT_PRIVATE MainLoop : public Udjat::MainLoop {
		private:

			enum MessageTypes : UINT {
				WM_CHECK_TIMERS			= WM_USER+101,
				WM_START				= WM_USER+102,
				WM_STOP					= WM_USER+103,
				WM_STOP_WITH_MESSAGE	= WM_USER+104,	///< @brief Terminate by console event or timer.
				WM_ADD_TIMER			= WM_USER+105,
				WM_REMOVE_TIMER			= WM_USER+106,
				WM_ADD_SOCKET			= WM_USER+107,
				WM_REMOVE_SOCKET		= WM_USER+108

			};

			/// @brief A class atom that uniquely identifies the class being registered.
			::ATOM identifier;

			MainLoop();

			class UDJAT_PRIVATE Timers {
			public:

				/// @brief Minimal timer value.
				unsigned long maxwait = 1000;

				/// @brief List of enabled timers.
				std::list<Timer *> enabled;

			};

			/// @brief Mutex
			static std::mutex guard;

			/// @brief Process windows messages.
			static LRESULT WINAPI hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			/// @brief Object window for this loop
			HWND hwnd = 0;

			/// @brief Current timer value, in milliseconds.
			UINT uElapse = 0;

			/// @brief get sockets
			ULONG getHandlers(WSAPOLLFD **fds, ULONG *length);

			/// @brief Active timers.
			Timers timers;

			/// @brief Active handlers.
			std::list<Handler *> handlers;

			/// @brief Run timers, compute poll timeout.
			/// @return The timeout to next 'poll()' call.
			unsigned long compute_poll_timeout() noexcept;

			/// @brief Process windows messages.
			static LRESULT WINAPI hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		public:
			virtual ~MainLoop();

			static MainLoop & getInstance();

			bool enabled(const Timer *timer) const noexcept override;

			void push_back(MainLoop::Timer *timer) override;
			void remove(MainLoop::Timer *timer) override;

			/// @brief Run mainloop.
			int run() override;

			/// @brief Wakeup main loop.
			void wakeup() noexcept override;

			/// @brief Quit mainloop.
			void quit() override;

			bool active() const noexcept override;

			BOOL post(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

			/// @brief Terminate with message
			/// @param message Message to show (Should be a constant to avoid 'out of scope' on message processing)
			void quit(const char *message) override;

			/// @brief Watch windows object.
			void insert(const void *id, HANDLE handle, const std::function<bool(HANDLE handle,bool abandoned)> call);

			static void insert(HANDLE handle, const std::function<bool(HANDLE handle,bool abandoned)> exec);
			static void remove(HANDLE handle);

			bool enabled(const Handler *handler) const noexcept override;

			void push_back(MainLoop::Handler *handler) override;
			void remove(MainLoop::Handler *handler) override;

			bool for_each(const std::function<bool(Timer &timer)> &func);

		};

	}

 }

/*
 #define WM_CHECK_TIMERS		WM_USER+101
 #define WM_START				WM_USER+102
 #define WM_STOP				WM_USER+103
 #define WM_STOP_WITH_MESSAGE	WM_USER+104	// Terminate by console event or timer
 #define WM_ADD_TIMER			WM_USER+105
 #define WM_REMOVE_TIMER		WM_USER+106

 #define IDT_CHECK_TIMERS	1

 using namespace std;

 namespace Udjat {

	namespace Win32 {

		class UDJAT_PRIVATE MainLoop : public Udjat::MainLoop {
		private:

			class UDJAT_PRIVATE Timers {
			public:

				/// @brief Minimal timer value.
				unsigned long maxwait = 1000;

				/// @brief List of enabled timers.
				std::list<Timer *> enabled;

			};

			/// @brief Mutex
			static std::mutex guard;

			/// @brief Process windows messages.
			static LRESULT WINAPI hwndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			/// @brief Object window for this loop
			HWND hwnd = 0;

			/// @brief Current timer value, in milliseconds.
			UINT uElapse = 0;

			/// @brief get sockets
			ULONG getHandlers(WSAPOLLFD **fds, ULONG *length);

			/// @brief Active timers.
			Timers timers;

			/// @brief Active handlers.
			std::list<Handler *> handlers;

			/// @brief Run timers, compute poll timeout.
			/// @return The timeout to next 'poll()' call.
			unsigned long compute_poll_timeout() noexcept;

		public:

			MainLoop();
			virtual ~MainLoop();

			bool enabled(const Timer *timer) const noexcept override;

			void push_back(MainLoop::Timer *timer) override;
			void remove(MainLoop::Timer *timer) override;

			/// @brief Run mainloop.
			int run() override;

			/// @brief Wakeup main loop.
			void wakeup() noexcept override;

			/// @brief Quit mainloop.
			void quit() override;

			bool active() const noexcept override;

			BOOL post(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

			/// @brief Terminate with message
			/// @param message Message to show (Should be a constant to avoid 'out of scope' on message processing)
			void quit(const char *message) override;

			/// @brief Watch windows object.
			void insert(const void *id, HANDLE handle, const std::function<bool(HANDLE handle,bool abandoned)> call);

			static void insert(HANDLE handle, const std::function<bool(HANDLE handle,bool abandoned)> exec);
			static void remove(HANDLE handle);

			bool enabled(const Handler *handler) const noexcept override;

			void push_back(MainLoop::Handler *handler) override;
			void remove(MainLoop::Handler *handler) override;

			bool for_each(const std::function<bool(Timer &timer)> &func);

		};


	}

 }
*/

