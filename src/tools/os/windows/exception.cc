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
 #include <private/misc.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/charset.h>
 #include <windows.h>
 #include <iostream>
 #include <mutex>

 #include <cstring>
 #include <cstdio>

 using namespace std;

 #define BUFFER_LENGTH 100

 class Guard : public mutex {
 public:
 	Guard() = default;

 	static Guard & getInstance() {
		static Guard instance;
		return instance;
 	}

 };

 static bool is_wine() noexcept {

	// https://stackoverflow.com/questions/7372388/determine-whether-a-program-is-running-under-wine-at-runtime

	HMODULE hntdll = GetModuleHandle("ntdll.dll");
	if(!hntdll) {
		return false;
	}

	return (((void *) GetProcAddress(hntdll, "wine_get_version")) != NULL);

 }

 std::string Udjat::Win32::Exception::format(const DWORD dwMessageId) noexcept {

	lock_guard<mutex> lock(Guard::getInstance());

	string response;
	char buffer[BUFFER_LENGTH+1];

	memset(buffer,0,BUFFER_LENGTH+1);

	SetLastError(0);

	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-makelangid
	int retval = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		0,
		dwMessageId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPTSTR) buffer,
		BUFFER_LENGTH,
		NULL
	);

	if(retval == 0) {

		auto winerror = GetLastError();

		response = "Windows error ";
		response += std::to_string((unsigned int) dwMessageId);

		if(winerror && winerror != ERROR_MR_MID_NOT_FOUND) {
			response += " (with an aditional error ";
			response += std::to_string(winerror);
			response += ")";
		}

	} else if(*buffer) {

		if(is_wine()) {

			response = buffer;

		} else {

			// TODO: Convert charset from windows.
			for(char *ptr = buffer; *ptr; ptr++) {
				if(*ptr < 0) {
					*ptr = '?';
				}
			}
			response = buffer;

		}

	} else {

		response = "Windows error ";
		response += std::to_string((unsigned int) dwMessageId);

	}

	SetLastError(dwMessageId);

	return response;

 }

 std::string Udjat::Win32::Exception::format(const char *what_arg, const DWORD dwMessageId) noexcept {
	 return string(what_arg) + " - " + format(dwMessageId).c_str();
 }

 std::string Udjat::Win32::WSA::Exception::format(const DWORD dwMessageId) noexcept {

	lock_guard<mutex> lock(Guard::getInstance());

	// https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2

	string response;
	char buffer[BUFFER_LENGTH+1];

	memset(buffer,0,BUFFER_LENGTH+1);

	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-makelangid
	int retval = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		0,
		dwMessageId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPTSTR) buffer,
		BUFFER_LENGTH,
		NULL
	);

	if(retval == 0 || !*buffer) {

		response = "WinSock error ";
		response += std::to_string((unsigned int) dwMessageId);
		response += " (check it in https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2)";

	} else {

		response = buffer;

	}

	return response;

 }

 std::string Udjat::Win32::WSA::Exception::format(const char *what_arg, const DWORD dwMessageId) noexcept {
	 return string(what_arg) + " - " + format(dwMessageId).c_str();
 }
