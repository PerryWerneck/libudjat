/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implements Udjat::Exception.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>

 #include <stdexcept>
 #include <string>
 #include <cstring>

 namespace Udjat {

	Udjat::Exception::Info::Info(int c, const char *t, const char *b, const char *u)
		: code{c}, title{t}, body{b}, url{u} {
	}

	Udjat::Exception::Info::Info(int c, const std::string &t, const std::string &b, const std::string &u)
		: code{c}, title{t}, body{b}, url{u} {
	}

	Udjat::Exception::Exception(int code, const char *message, const char *body, const char *url)
		: std::runtime_error(message), info{code,_("Operation has failed"),body,url} {
	}

	Udjat::Exception::Exception(int code, const std::string &message, const std::string &body, const std::string &url)
		: std::runtime_error(message), info{code,_("Operation has failed"),body,url} {
	}

	Udjat::Exception::Exception(int code, const char *message, const std::string &body, const std::string &url)
		: std::runtime_error(message), info{code,_("Operation has failed"),body,url} {
	}

	Udjat::Exception::Exception(const char *message, const char *body)
		:	Exception{-1,message,body} {
	}

	Udjat::Exception::Exception(int code)
		:	std::runtime_error{strerror(code)}, info{code,_("System Error"),""} {
	}

 }

