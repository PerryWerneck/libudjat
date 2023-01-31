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
 #include <udjat/tools/string.h>
 #include <windows.h>
 #include <iostream>
 #include <mutex>
 #include <wininet.h>
 #include <comdef.h>

 #include <cstring>
 #include <cstdio>

 using namespace std;

 #define BUFFER_LENGTH 1024

 /*
 static const struct {
	DWORD dwMessageId;
	int err;
 } system_errors[] = {
	{	}
 }
 */

 static const struct {
	DWORD dwMessageId;
	const char *message;
 } windows_errors[] = {

 	// /usr/x86_64-w64-mingw32/sys-root/mingw/include/wininet.h
 	{ ERROR_INTERNET_TIMEOUT,	N_("The request has timed out")	},

 	// http://s.web.umkc.edu/szb53/cs423_sp16/wsock_errors.html
 	// /usr/x86_64-w64-mingw32/sys-root/mingw/include/winerror.h
	{ WSAHOST_NOT_FOUND,		N_("Cant resolve hostname")			},

 };


 namespace Udjat {

	 namespace Win32 {

		class UDJAT_PRIVATE Guard : public mutex {
		public:
			Guard() = default;

			static Guard & getInstance() {
				static Guard instance;
				return instance;
			}

		};

		std::string Exception::format(const char *what_arg, const DWORD dwMessageId) noexcept {
			 return string(what_arg) + " - " + format(dwMessageId).c_str();
		}

		std::string Exception::format(const DWORD dwMessageId) noexcept {

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

			if(retval == 0 || !*buffer) {

				for(size_t ix = 0; ix < N_ELEMENTS(windows_errors); ix++) {
					if(windows_errors[ix].dwMessageId == dwMessageId) {
		#ifdef GETTEXT_PACKAGE
						return dgettext(GETTEXT_PACKAGE,windows_errors[ix].message);
		#else
						return windows_errors[ix].message;
		#endif // GETTEXT_PACKAGE
					}
				}
			}

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

				response = Win32::Charset::from_windows(strip(buffer));

			} else {

				response = "Windows error ";
				response += std::to_string((unsigned int) dwMessageId);

			}

			SetLastError(dwMessageId);

			return response;

		}

		namespace COM {

			class Error : public _com_error, public std::exception, public std::string {
			public:
				Error(const HRESULT result) : _com_error{result} {

					std::string::assign(
						Win32::Charset::from_windows(
							_com_error::ErrorMessage()
						)
					);

					cerr << "win32\t" << std::string::c_str() << endl;

				}

				const char * what() const noexcept override {
					return std::string::c_str();
				}

			};

		}

		void UDJAT_API throw_if_fail(const DWORD error) {

			// https://learn.microsoft.com/en-us/windows/win32/learnwin32/error-handling-in-com#throw-on-fail

			if(error) {

				// TODO: Check if 'error' can be translated to std::system_error.

				throw Exception(error);
			}

		 }

		void UDJAT_API throw_if_fail(const HRESULT result) {
			if (FAILED(result)) {
				throw COM::Error(result);
			}
		 }

		void UDJAT_API throw_if_fail(const char *str, const DWORD error) {

			if(error) {

				// TODO: Check if 'error' can be translated to std::system_error.

				throw Win32::Exception(str, error);
			}

		 }

		namespace WSA {

			std::string Exception::format(const char *what_arg, const DWORD dwMessageId) noexcept {
				 return string(what_arg) + " - " + format(dwMessageId).c_str();
			}

			std::string Exception::format(const DWORD dwMessageId) noexcept {

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

					for(size_t ix = 0; ix < N_ELEMENTS(windows_errors); ix++) {
						if(windows_errors[ix].dwMessageId == dwMessageId) {
			#ifdef GETTEXT_PACKAGE
							return dgettext(GETTEXT_PACKAGE,windows_errors[ix].message);
			#else
							return windows_errors[ix].message;
			#endif // GETTEXT_PACKAGE
						}
					}
				}

				if(retval == 0 || !*buffer) {

					response = "WinSock error ";
					response += std::to_string((unsigned int) dwMessageId);
					response += " (check it in https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2)";

				} else if(*buffer) {

					response = Win32::Charset::from_windows(strip(buffer));

				}

				return response;

			}

		}


	 }

 }

 /*
 static bool is_wine() noexcept {

	// https://stackoverflow.com/questions/7372388/determine-whether-a-program-is-running-under-wine-at-runtime
	static unsigned char detected = 3;

	if(detected == 3) {
		HMODULE hntdll = GetModuleHandle("ntdll.dll");
		if(!hntdll) {
			return false;
		}
		detected = GetProcAddress(hntdll, "wine_get_version") == NULL ? 0 : 1;
	}

	return detected != 0;

 }
 */


