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
 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/error.h>
 #include <system_error>
 #include <cstring>
 #include <string>
 #include <iostream>
 #include <errno.h>

 using namespace std;

 namespace Udjat {

	static string check_message(HTTP::StatusCode code, const char *message = nullptr) {
		if(message && *message) {
			return message;
		}
		if((int) code == ECANCELED) {
			return strerror((int) code);
		}
		return std::to_string(code);
	}

	HTTP::Exception::Exception(StatusCode code) 
		: runtime_error{check_message(code)} {
	}

	HTTP::Exception::Exception(StatusCode code, const char *message)
		: runtime_error{check_message(code,message)} {

	}
	HTTP::Exception::Exception(const char *message)
		: runtime_error{check_message(HTTP::SystemError,message)} {
	}

 }
