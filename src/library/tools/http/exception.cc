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
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/error.h>
 #include <system_error>
 #include <cstring>
 #include <string>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	// https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
	static const struct {
		int http;
		int syscode;
	} syscodes[] = {
		{ 200, 0			},
		{ 401, EPERM 		},
		{ 403, EPERM 		},
		{ 404, ENOENT		},
		{ 405, EINVAL		},
		{ 407, EPERM	 	},
#ifdef ETIMEDOUT
		{ 408, ETIMEDOUT 	},
#endif // ETIMEDOUT
		{ 501, ENOTSUP		},
		{ 503, EBUSY	 	},
		{ 404, ENODATA		},
	};

	int HTTP::Exception::code(int syscode) noexcept {
		for(const auto &code : syscodes) {
			if(code.syscode == syscode) {
				return code.http;
			}
		}
		return 500;
	}

	int HTTP::Exception::syscode(unsigned int httpcode) noexcept {
		for(const auto &code : syscodes) {
			if((unsigned int) code.http == httpcode) {
				return code.syscode;
			}
		}
		return -1;
	}

	int HTTP::Exception::code(const system_error &except) noexcept {
		return code(except.code().value());
	}


	HTTP::Exception::Exception(unsigned int hc)
		: Udjat::Exception{syscode(hc)}, http_code{hc} {

	}

	static string check_message(unsigned int hc, const char *message) {
		if(message && *message) {
			return message;
		}
		if(hc == ECANCELED) {
			return strerror(hc);
		}
		return Logger::Message{"HTTP error {}",hc};
	}

	HTTP::Exception::Exception(unsigned int hc, const char *message)
		: Udjat::Exception{syscode(hc),check_message(hc,message).c_str()}, http_code{hc} {

	}
	HTTP::Exception::Exception(const char *message)
		: Udjat::Exception{check_message(500,message).c_str()}, http_code{500} {

	}


 }
