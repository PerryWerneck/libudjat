/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Declare api call.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/abstract/response.h>
 #include <string>

 namespace Udjat {

	/// @brief Convenience class for customized API calls.
	class UDJAT_API RequestPath {
	private:
		const char *object_name;
		const char *object_path;
		HTTP::Method method = HTTP::Get;

	protected:

		/// @brief Expiration time (for http responses, default is 1 minute)
		/// @see Abstract::Response::expires
		time_t expires;

		/// @brief Check/update cache information.
		/// @return true The api is cached, no need to continue.
		bool head(const Request &request, Abstract::Response &response) const;

	public:
		RequestPath(const XML::Node &node);

		inline const char *name() const noexcept {
			return object_name;
		}

		inline const char *path() const noexcept {
			return object_path;
		}

		inline operator HTTP::Method() const noexcept {
			return method;
		}

		inline bool operator==(HTTP::Method m) const noexcept {
			return m == method;
		}

		/// @brief Test if path begins with this object path.
		/// @param path The path to test.
		/// @return true if the beginning of path is the same of this object.
		bool operator==(const char *path) const noexcept;

		bool operator==(const Request &Request) const noexcept;

	};

 }
