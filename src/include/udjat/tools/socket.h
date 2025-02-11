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
	private:
		bool connecting = false;
	
	protected:
		void handle_event(const Event event) override;

		/// @brief Handle connection to URL
		/// @param error_code The error code, non zero if connection has failed and socket is now closed.
		virtual void handle_connect(int error_code);
		
		virtual void handle_disconnect();
		virtual void handle_read_ok();
		virtual void handle_write_ok();
		virtual void handle_error(int code);

	public:

		/// @brief Connect to URL
		/// @param url URL to connect
		Socket(const URL &url, unsigned int seconds = 0);

		/// @brief Handle connected socket.
		/// @param fd The socket to handle.
		Socket(int fd);

		virtual ~Socket();

		static void blocking(int sock, bool enable = true);

		inline void blocking(bool enable) {
			blocking(values.fd,enable);
		}

		static void close(int sock) noexcept;

		void close() override;

		/// @brief Wait for the connection to establish.
		/// @param timeout Timeout in milliseconds.
		/// @return sock if the connection is established, -1 otherwise (set errno).
		static int wait_for_connection(int sock, unsigned int seconds = 0);

		inline int wait_for_connection(unsigned int seconds = 0) {
			return wait_for_connection(values.fd, seconds);
		}

	};

 }

