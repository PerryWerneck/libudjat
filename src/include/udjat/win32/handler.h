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

 namespace Udjat {

	namespace Win32 {

		/// @brief Windows event handler.
		class UDJAT_API Handler {
		protected:

			/// @brief The event handle.
			HANDLE hEvent = 0;

			/// @brief Start event watcher.
			void start();

		public:

			class Controller;
			friend class controller;

			constexpr Handler() {
			}

			constexpr Handler(HANDLE handle) : hEvent(handle) {
			}

			operator bool() const noexcept {
				return hEvent != 0;
			}

			virtual ~Handler();

			void enable();
			void disable();
			void set(HANDLE handle);
			void close();
			ssize_t read(void *buf, size_t count);

			/// @brief Handle activity.
			virtual void handle(bool abandoned) = 0;

			/// @brief Wait for handlers.
			/// @param handlers	List of handlers to poll.
			/// @param nfds Length of 'handlers'.
			/// @param timeout for poll.
			/// @return Count of valid handlers (0=none).
			static size_t poll(Win32::Handler **handlers, size_t nfds, int timeout);

		};


	}

 }
