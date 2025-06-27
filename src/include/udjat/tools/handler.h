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

 #ifndef _WIN32
	#include <sys/poll.h>
 #endif // _WIN32

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

	protected:

		struct Values {
			int fd = -1;
			Event events = (Event) 0;

			constexpr Values(int f, Event e) : fd{f}, events{e} {
			}

			constexpr Values() {
			}

		} values;

		virtual void handle_event(const Event event) = 0;

	public:

		constexpr Handler() { };

		Handler(int fd, const Event event);
		virtual ~Handler();

		/// @brief Set file handler to new value.
		/// @return true if the handler was changed.
		bool set(int fd);
		void set(const Event events);

		virtual void flush();

		void handle(const Event event) noexcept;

#ifndef _WIN32
		inline void get(pollfd &pfd) const noexcept {
			pfd.fd = values.fd;
			pfd.events = values.events;
			pfd.revents = 0;
		}

		inline void set(pollfd &pfd) {
			handle_event((Event) pfd.revents);
		}
#endif // _WIN32

		inline Handler & fd(int v) noexcept {
			set(v);
			return *this;
		}

		inline Handler & operator = (int fd) {
			set(fd);
			return *this;
		}

		inline Handler & operator = (const Event events) {
			set(events);
			return *this;
		}

		inline operator bool() const noexcept {
			return values.fd != -1;
		}

		inline operator int() const noexcept {
			return values.fd;
		}

		inline int fd() const noexcept {
			return values.fd;
		}

		inline Event events() const noexcept {
			return values.events;
		}

		/// @brief Is handler enabled?
		bool enabled() const noexcept;

		/// @brief Enable handler.
		/// @return false if the handler was already enabled.
		bool enable() noexcept;

		/// @brief Disable handler.
		void disable() noexcept;

		ssize_t read(void *buf, size_t count);

		virtual void close();

		/// @brief Process handlers.
		/// @param handlers	List of handlers to poll.
		/// @param nfds Length of 'handlers'.
		/// @param timeout for poll.
		/// @return Count of valid handlers (0=none).
		static size_t poll(Handler **handlers, size_t nfds, int timeout);

		/// @brief Handle pending events.
		/// @param handlers	List of handlers to poll.
		/// @param nfds Length of 'handlers'.
		/// @param timeout for poll.
		/// @return Count of valid handlers (0=none).
		static size_t flush(Handler **handlers, size_t nfds, int timeout);

	};



 }

