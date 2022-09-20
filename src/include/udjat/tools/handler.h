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

 namespace Udjat {

	///< @brief File/Socket handler
	class UDJAT_API MainLoop::Handler {
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


	private:

#ifndef _WIN32
		friend class MainLoop;
		int index=-1;
#endif // _WIN32

	protected:

		int fd = -1;
		Event events = (Event) 0;
		virtual void handle_event(const Event event) = 0;

	public:

		constexpr Handler() { };

		Handler(int fd, const Event event);
		virtual ~Handler();

		void set(int fd);

		void set(const Event events);

		inline Handler & operator = (int fd) {
			set(fd);
			return *this;
		}

		inline Handler & operator = (const Event events) {
			set(events);
			return *this;
		}

		inline operator bool() const noexcept {
			return fd != -1;
		}

		inline operator int() const noexcept {
			return fd;
		}

		/// @brief Is handler enabled?
		bool enabled() const noexcept;

		/// @brief Enable handler.
		/// @return false if the handler was already enabled.
		bool enable() noexcept;

		/// @brief Disable handler.
		void disable() noexcept;

		ssize_t read(void *buf, size_t count);

		void close();

		/// @brief Wait for handlers.
		/// @param handlers	List of handlers to poll.
		/// @param nfds Length of 'handlers'.
		/// @param timeout for poll.
		/// @return Count of valid handlers (0=none).
		static size_t poll(Handler **handlers, size_t nfds, int timeout);

	};



 }

