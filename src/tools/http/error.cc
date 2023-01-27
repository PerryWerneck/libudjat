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

 /*
  * References:
  *
  * https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
  * https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
  *
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/intl.h>
 #include <udjat/agent/state.h>
 #include <udjat/tools/http/error.h>
 #include <udjat/agent/level.h>

 using namespace std;

 namespace Udjat {

	static const struct {
		const char *name;
		int32_t from;
		int32_t to;
		Udjat::Level level;
		const char * summary;
		const char * body;
	} error_codes[] = {

		//
		// Informational responses (100 – 199)
		//
		{
			"Ok",
			100,199,
			Udjat::ready,
			N_("Information"),
			""
		},

		//
		// Successful responses (200 – 299)
		//
		{
			"Ok",
			200,299,
			Udjat::ready,
			N_("Ok"),
			N_("The request succeeded.")
		},

		//
		// Redirection messages (300 – 399)
		//
		{
			"Ok",
			300,399,
			Udjat::ready,
			N_("Redirected"),
			N_("The request was redirected to another URL.")
		},

		//
		// Client error responses (400 – 499)
		//
		{
			"BadRequest",
			400,400,
			Udjat::warning,
			N_("Bad Request"),
			N_("The server cannot or will not process the request due to an apparent client error (e.g., malformed request syntax, size too large, invalid request message framing, or deceptive request routing).")
		},
		{
			"Unauthorized",
			401,401,
			Udjat::warning,
			N_("Unauthorized"),
			N_("Authentication is required and has failed or has not yet been provided.")
		},
		{
			"Forbidden",
			403,403,
			Udjat::warning,
			N_("Forbidden"),
			N_("The request contained valid data and was understood by the server, but the server is refusing action.")
		},
		{
			"NotFound",
			404,404,
			Udjat::error,
			N_("Not found"),
			N_("The requested resource could not be found but may be available in the future.")
		},
		{
			"Proxy",
			407,407,
			Udjat::error,
			N_("Proxy Authentication Required"),
			N_("The client must first authenticate itself with the proxy.")
		},
		{
			"Timeout",
			408,408,
			Udjat::error,
			N_("Request Timeout"),
			N_("The client did not produce a request within the time that the server was prepared to wait.")
		},
		{
			"Gone",
			410,410,
			Udjat::error,
			N_("Gone"),
			N_("The resource requested was previously in use but is no longer available and will not be available again.")
		},
		{
			"Overload",
			429,429,
			Udjat::error,
			N_("Too many requests"),
			N_("Something is making your server work too hard, and it just can’t keep up")
		},
		{
			"ClientError",
			400,499,
			Udjat::ready,
			N_("Client error"),
			""
		},

		//
		// Server error responses (500 – 599)
		//
		{
			"Internal",
			500,500,
			Udjat::error,
			N_("Internal error"),
			N_("The server has encountered a situation it does not know how to handle.")
		},
		{
			"NotImplemented",
			501,501,
			Udjat::error,
			N_("Not Implemented"),
			N_("The request method is not supported by the server and cannot be handled.")
		},
		{
			"BadGateway",
			502,502,
			Udjat::error,
			N_("Bad gateway"),
			N_("The server, while working as a gateway to get a response needed to handle the request, got an invalid response.")
		},
		{
			"Unavailable",
			503,503,
			Udjat::error,
			N_("Service Unavailable"),
			N_("The server is not ready to handle the request. Common causes are a server that is down for maintenance or that is overloaded.")
		},
		{
			"Timeout",
			504,504,
			Udjat::error,
			N_("Gateway timeout"),
			N_("The server is acting as a gateway and cannot get a response in time.")
		},
		{
			"BadVersion",
			505,505,
			Udjat::error,
			N_("HTTP Version Not Supported"),
			N_("The HTTP version used in the request is not supported by the server.")
		},
		{
			"ServerError",
			500,599,
			Udjat::error,
			N_("Server error"),
			""
		},
	};

	HTTP::Error HTTP::Error::Factory(int32_t code) {
		for(size_t ix = 0; ix < N_ELEMENTS(error_codes);ix++) {
			if(code >= error_codes[ix].from && code <= error_codes[ix].to) {
				return HTTP::Error{error_codes[ix].level,error_codes[ix].summary,error_codes[ix].body};
			}
		}
		return HTTP::Error{Udjat::critical,_("Unexpected HTTP error code"),_("The HTTP error code is unknown")};
	}

	const char * HTTP::Error::to_string() const noexcept {
		if(body && *body) {
			return body;
		}
		return summary;
	}

	std::shared_ptr<Abstract::State> HTTP::Error::StateFactory(int32_t code) {

		for(size_t ix = 0; ix < N_ELEMENTS(error_codes);ix++) {

			if(code >= error_codes[ix].from && code <= error_codes[ix].to) {
				return make_shared<Abstract::State>(
						error_codes[ix].name,
						error_codes[ix].level,
#ifdef GETTEXT_PACKAGE
						dgettext(GETTEXT_PACKAGE,error_codes[ix].summary),
						dgettext(GETTEXT_PACKAGE,error_codes[ix].body)
#else
						error_codes[ix].summary,
						error_codes[ix].body
#endif // GETTEXT_PACKAGE
					);

			}

		}

		return std::shared_ptr<Abstract::State>();

	}

 }
