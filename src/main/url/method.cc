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

 #include "private.h"
 #include <cstring>

 namespace Udjat {

	static const char *names[] = {
		"GET",		// Requests a representation of the specified resource.
		"HEAD",		// Asks for a response identical to that of a GET request, but without the response body.
		"POST",		// Submit an entity to the specified resource, often causing a change in state or side effects on the server.
		"PUT",		// Replaces all current representations of the target resource with the request payload.
		"DELETE",	// Deletes the specified resource.
		"CONNECT",	// Establishes a tunnel to the server identified by the target resource.
		"OPTIONS",	// Describe the communication options for the target resource
		"TRACE",	// Performs a message loop-back test along the path to the target resource.
		"PATCH",	// Apply partial modifications to a resource.
	};

	static URL::Method::Value value_from_string(const char *value) {

		for(uint8_t ix = 0; ix < (sizeof(names)/sizeof(names[0]));ix++) {
			if(!strcasecmp(names[ix],value)) {
				return (URL::Method::Value) ix;
			}
		}

		throw runtime_error(string{"I don't know nothing about the URL method '"} + value + "'");

	}

	URL::Method::Method(const char *name) : Method(value_from_string(name)) {
	}

	URL::Method & URL::Method::operator = (const char *name) {
		value = value_from_string(name);
		return *this;
	}

	const char * URL::Method::c_str() const noexcept {

		if(value > (sizeof(names)/sizeof(names[0])))
			return "";

		return names[(int) value];
	}


 }

