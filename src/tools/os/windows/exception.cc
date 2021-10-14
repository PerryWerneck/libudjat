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

 #include <config.h>
 #include <udjat-internals.h>
 #include <udjat/win32/exception.h>
 #include <windows.h>

 #include <cstring>
 #include <cstdio>

 using namespace std;

 std::string Win32::Exception::format(const char *what_arg, const DWORD error) noexcept {

	string response{what_arg};
	LPVOID lpMsgBuf = 0;

	if(FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL) == 0) {

		if(lpMsgBuf) {

			char *ptr;
			for(ptr=(char *) lpMsgBuf;*ptr && *ptr != '\n' && *ptr != '\r';ptr++);
			*ptr = 0;

			response += " - ";
			response += (const char *) lpMsgBuf;

			LocalFree(lpMsgBuf);

			// TODO: Convert response to UTF-8

		} else {
			char msg[4096];
			snprintf(msg,4095," - The windows error was %u - 0x%08x",(unsigned int) error, (unsigned int) error);
			response += msg;
		}

	}  else {

		char msg[4096];
		snprintf(msg,4095," - The windows error was %u - 0x%08x",(unsigned int) error, (unsigned int) error);
		response += msg;

	}

	return response;

}
