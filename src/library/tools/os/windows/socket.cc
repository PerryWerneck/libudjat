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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/socket.h>
 #include <udjat/win32/exception.h>
 #include <winsock.h>

 using namespace std;

 namespace Udjat {

	void Socket::set_blocking(int sock, bool blocking) {

		u_long mode = (blocking ? 1 : 0); 
		if(ioctlsocket(sock, FIONBIO, &mode) != 0) {
			throw Win32::Exception();
		}

	}

	void Socket::set_timeout(int sock, unsigned int seconds) {

		// Reference: https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout

		// WINDOWS
		DWORD timeout = seconds * 1000;

		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof timeout);

	}

 }

