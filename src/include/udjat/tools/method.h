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

 namespace Udjat {

	namespace HTTP {

		/// @brief Protocol Method.
		/// <https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods>
		enum Method : uint8_t {
			Get,		///< @brief Requests a representation of the specified resource.
			Head,		///< @brief Asks for a response identical to that of a GET request, but without the response body.
			Post,		///< @brief Submit an entity to the specified resource, often causing a change in state or side effects on the server.
			Put,		///< @brief Replaces all current representations of the target resource with the request payload.
			Delete,		///< @brief Deletes the specified resource.
			Connect,	///< @brief Establishes a tunnel to the server identified by the target resource.
			Options,	///< @brief Describe the communication options for the target resource.
			Trace,		///< @brief Performs a message loop-back test along the path to the target resource.
			Patch,		///< @brief Apply partial modifications to a resource.
		};

		UDJAT_API Method MethodFactory(const char *name);

	}

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::HTTP::Method method);

	inline ostream& operator<< (ostream& os, const Udjat::HTTP::Method method) {
		return os << to_string(method);
	}

 }
