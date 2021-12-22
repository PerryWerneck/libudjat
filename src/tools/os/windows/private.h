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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/configuration.h>
 #include <iostream>

 #ifdef _WIN32
	#include <udjat/win32/exception.h>
 #endif // _WIN32

 #ifdef HAVE_WINHTTP
	#include <udjat/tools/http.h>
 	#include <winhttp.h>
 #endif // HAVE_WINHTTP

 using namespace std;

 namespace Udjat {

#ifdef HAVE_WINHTTP
	namespace HTTP {

		class Client::Worker {
		private:
			HTTP::Client *client;

			/// @brief WinHTTP session handle.
			HINTERNET session;

			/// @brief Connect to HTTP host.
			HINTERNET connect();

			/// @brief Open HTTP Request.
			HINTERNET open(HINTERNET connection, const LPCWSTR pwszVerb);

			/// @brief Send request.
			void send(HINTERNET request, const char *payload = nullptr);

			/// @brief Wait for response.
			std::string wait(HINTERNET req);

			Worker(HTTP::Client *s);

		public:
			~Worker();

			static Worker * getInstance(HTTP::Client *client);

			std::string call(const char *verb, const char *payload = nullptr);

			/*
			std::string get();
			std::string post();
			*/

		};

	}
#endif // HAVE_WINHTTP

 }

