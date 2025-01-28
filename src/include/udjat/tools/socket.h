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

 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/url.h>

 namespace Udjat {

	class UDJAT_API Socket : public MainLoop::Handler {
	protected:
		int fd = -1;

		void handle_event(const Event event) noexcept override;

		virtual void handle_connect() noexcept;
		virtual void handle_disconnect(int code) noexcept;
		virtual void handle_read() noexcept;
		virtual void handle_error(int code) noexcept;

	public:

		/// @brief Connect to URL
		/// @param url URL to connect
		Socket(const URL &url);

		/// @brief Handle connected socket.
		/// @param fd The socket to handle.
		Socket(int fd);

		virtual ~Socket();

		void close();

	};

 }

