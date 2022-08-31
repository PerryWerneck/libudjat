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

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/misc.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/win32/event.h>
 #include <thread>
 #include <list>
 #include <mutex>

 using namespace std;

 namespace Udjat {

	namespace Win32 {

		class UDJAT_API Event::Controller {
		public:
			struct Worker {
				bool enabled = true;
				std::thread *hThread = nullptr;
				std::list<Win32::Event *> events;

				// Disable copy.
				Worker(const Worker &) = delete;
				Worker(const Worker *) = delete;

				Worker(Win32::Event *event);

				~Worker();

			};

			static Win32::Event * find(Worker *worker, HANDLE handle) noexcept;

			static bool wait(Worker *worker) noexcept;

		private:

			std::list<Worker> workers;

			static std::mutex guard;

			Controller() = default;
		public:
			static Controller & getInstance();

			void insert(Event *event);
			void remove(Event *event);

			Win32::Event * find(HANDLE handle) noexcept;

		};


		/// @brief Win32 Mainloop for console application.
		class MainLoop : public Udjat::MainLoop {
		private:
			static BOOL WINAPI ConsoleHandler(DWORD event);
			MainLoop();

		public:
			~MainLoop();

			static MainLoop & getInstance();

		};


	}

 }
