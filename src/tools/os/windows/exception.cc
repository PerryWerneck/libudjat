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
 #include <iostream>

 #include <cstring>
 #include <cstdio>

 using namespace std;

 std::string Udjat::Win32::Exception::format(const DWORD dwMessageId) noexcept {

	string response;
	LPVOID lpMsgBuf = 0;

	DWORD szMessage =
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_MAX_WIDTH_MASK,
			0,
			dwMessageId,
			0,
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
		);

	if(lpMsgBuf) {

		if(szMessage) {
			response = string((const char *) lpMsgBuf,szMessage);
		}
	
		LocalFree(lpMsgBuf);

	} else {

		if(szMessage == 0) {
			cerr << "win32\tError " << GetLastError() << " when formatting message id " << dwMessageId << endl;
		}

		response = "The windows error was ";
		response += std::to_string((unsigned int) dwMessageId);
	}

	return response;

 }

 std::string Udjat::Win32::Exception::format(const char *what_arg, const DWORD dwMessageId) noexcept {
	 return string(what_arg) + " - " + format(dwMessageId).c_str();
 }
