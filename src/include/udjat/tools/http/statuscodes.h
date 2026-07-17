/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <string>

 namespace Udjat {

	namespace HTTP {

		// Reference: https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status
		enum StatusCode : unsigned short {
			Undefined				= 0,	///< @brief Status code is undefined.
			Ok 						= 200,	///< @brief The request succeeded. The result and meaning of "success" depends on the HTTP method
			NoContent				= 204,	///< @brief There is no content to send for this request, but the headers are useful.
			NotModified				= 304,
			BadRequest				= 400,	///< @brief The server cannot or will not process the request due to something that is perceived to be a client error
			UnAuthenticated			= 401,
			Forbidden				= 403,
			NotFound				= 404,
			MethodNotAllowed		= 405,	///< @brief The request method is known by the server but is not supported by the target resource.
			ProxyAuthRequired		= 407,
			RequestTimeout			= 408,
			Gone 					= 410, 	///< @brief This response is sent when the requested content has been permanently deleted from server.
			Unprocessable			= 402,	///< @brief The request was well-formed but was unable to be followed due to semantic errors.
			UnsupportedMediaType	= 415,	///< @brief The media format of the requested data is not supported by the server.
			SystemError				= 500,	///< @brief Internal Server Error.
			NotImplemented			= 501,	///< @brief The request method is not supported by the server and cannot be handled.
			Unavailable				= 503,	///< @brief The server is not ready to handle the request.
		};

	}

 }

 namespace std {

	UDJAT_API const string to_string(const Udjat::HTTP::StatusCode status);

 }
