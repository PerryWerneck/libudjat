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
  * @brief Declares the abstract interface for API calls.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/authentication.h>
 #include <udjat/tools/http/statuscodes.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/http/status.h>
 #include <cstring>
 #include <ostream>

 namespace Udjat {

	/// @brief Interface for processing requests.
	class UDJAT_API Interface {
	private:
		const char *interface_name;
		const Authentication::Role role;

	protected:
		typedef Interface super;

		/// @brief Check if the request can be processed.
		/// @param[in] path The path for required object on interface.
		/// @param request The request to be validated.
		/// @param response The response to receive the status code and error message.
		/// @return true if the request can bem processed.
		bool allow(const char *path, const Request &request, Response &response) const noexcept;

	public:
		Interface(const char *name, const Authentication::Role = Authentication::Admin);
		
		virtual ~Interface();

		inline const char *name() const noexcept {
			return interface_name;
		}

		inline const char *c_str() const noexcept {
			return interface_name;
		}

#if __cplusplus >= 202002L
		inline auto operator <=>(const char *name) const noexcept {
			return strcasecmp(name,this->_name);
		}
#else
		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(name,interface_name) == 0;
		}
#endif

		/// @brief Check the required role for this interface.
		/// @param role The current user role.
		/// @return true if the user has access to this interface.
		inline bool allow(const Authentication::Role role = Authentication::None) const noexcept {
			return role >= this->role;
		}

		/// @brief Retrieves the schema definition for the interface inputs.
		/// @param[in] path The path for required object on interface.
		/// @param[out] s Object populated with the interface input schema details.
		/// @return True if the interface defines an input schema; false otherwise (schema remains unmodified).
		virtual bool schema(const char *path, HTTPSchema &schema) const noexcept;

		/// @brief Retrieves the schema definition for the interface inputs.
		/// @param[in] method The requested method.
		/// @param[in] path The path for required object on interface.
		/// @param[out] s Object populated with the interface input schema details.
		/// @return True if the interface defines an input schema; false otherwise (schema remains unmodified).
		virtual bool schema(const HTTP::Method method, const char *path, InputSchema &schema) const noexcept;

		inline bool schema(const char *path, InputSchema &schema) const noexcept {
			return this->schema(HTTP::Get,path,schema);
		}

		inline bool schema(InputSchema &s) const noexcept {
			return schema(HTTP::Get,"",s);
		}

		/// @brief Retrieves the schema definition for the interface outputs.
		/// @param[in] method The requested method.
		/// @param[in] path The path for required object on interface.
		/// @param[out] schema Object populated with the interface output schema details.
		/// @return True if the interface defines an output schema; false otherwise (schema remains unmodified).
		virtual bool schema(const HTTP::Method method, const char *path, OutputSchema &schema) const noexcept;

		/// @brief Retrieves the schema definition for 'get' requests.
		/// @param[in] path The path for required object on interface.
		/// @param[out] schema Object populated with the interface output schema details.
		/// @return True if the interface defines an output schema; false otherwise (schema remains unmodified).
		inline bool schema(const char *path, OutputSchema &schema) const noexcept {
			return this->schema(HTTP::Get,path,schema);
		}

		/// @brief Retrieves the schema definition for default 'get' requests.
		/// @param[out] schema Object populated with the interface output schema details.
		/// @return True if the interface defines an output schema; false otherwise (schema remains unmodified).
		inline bool schema(OutputSchema &s) const noexcept {
			return schema(HTTP::Get,"",s);
		}

		/// @brief Process an API request.
		/// @param path The request path.
		/// @param request The client request.
		/// @param response The expected response.
		/// @return true if the request was recognized and processed.
		virtual bool process(const Request &request, Response &response) const noexcept;

		/// @brief Process a stream request (usually from HTTP server);
		/// @param path The request path.
		/// @param request The client request.
		/// @param status Object to receive the processing status.
		/// @param stream The output stream to receive the interface section.
		/// @return true if the request was recognized and processed.
		virtual bool process(const Request &request, HTTP::Status &status, std::ostream &stream) const noexcept;

		/// @brief Enumerate interfaces.
		static bool for_each(const std::function<bool(const Interface &interface)> &func);

		/// @brief Find an interface matching with path, extract prefix.
		/// @param path The path for required interface, if found the interface part will be stripped.
		/// @return A valid interface, nullptr if not found.
		static Interface * find(const char * &path) noexcept;

		/// @brief Find an interface matching with request, extract prefix.
		/// @param request The request, if found the interface part will be 'popped' out from request.
		/// @return A valid interface, nullptr if not found.
		static Interface * find(Request &request) noexcept;

	};

 }
