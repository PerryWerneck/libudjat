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
 #include <udjat/win32/handler.h>
 #include <thread>
 #include <list>
 #include <mutex>

 using namespace std;

 namespace Udjat {

	namespace Win32 {

		class UDJAT_API Handler::Controller {
		public:

			/// @brief Object to manage a list of handlers.
			class Worker {
			private:
				~Worker();	// Will be deleted when worker thread finishes.

			public:
				Worker(Win32::Handler *handler);

				std::list<Win32::Handler *> handlers;

				// Disable copy.
				Worker(const Worker &) = delete;
				Worker(const Worker *) = delete;

			};

			static Win32::Handler * find(Worker *worker, HANDLE handle) noexcept;

			bool wait(Worker *worker) noexcept;
			void call(HANDLE handle, bool abandoned) noexcept;

		private:

			std::list<Worker *> workers;

			static std::mutex guard;

			Controller();
			~Controller();

		public:
			static Controller & getInstance();

			void insert(Handler *handler);
			void remove(Handler *handler);

			Win32::Handler * find(HANDLE handle) noexcept;

		};

	}

 }
