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
		/// @param[out] schema Object populated with the interface input schema details.
		/// @return True if the interface defines an input schema; false otherwise (schema remains unmodified).
		virtual bool input_schema(Schema &schema) const noexcept;

		/// @brief Retrieves the schema definition for the interface outputs.
		/// @param[out] schema Object populated with the interface output schema details.
		/// @return True if the interface defines an output schema; false otherwise (schema remains unmodified).
		virtual bool output_schema(Schema &schema) const noexcept;

		/// @brief Process an API request.
		/// @param path The request path.
		/// @param request The client request.
		/// @param response The expected response.
		/// @return true if the request was recognized and processed.
		virtual bool process(const char *path, const Request &request, Response &response) const;

		/// @brief Process a stream request (usually from HTTP server);
		/// @param path The request path.
		/// @param request The client request.
		/// @param stream The output stream.
		/// @return true if the request was recognized and processed.
		virtual bool process(const char *path, const Request &request, std::ostream &stream) const;

		/// @brief Enumerate interfaces.
		static bool for_each(const std::function<bool(const Interface &interface)> &func);

	};

 }
