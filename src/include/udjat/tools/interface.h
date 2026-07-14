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
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/schema.h>
 #include <udjat/action.h>
 #include <udjat/authentication.h>
 #include <vector>
 #include <memory>
 #include <vector>
 #include <functional>

 namespace Udjat {

	/// @brief Abstract handler.
	class UDJAT_API Interface {
	public:

		/// @brief A request handler method.
		class UDJAT_API Handler {
		public:

			Handler(const char *name = "unnamed");
			Handler(const char *name, const Properties &props);
			Handler(const Properties &props);

			virtual ~Handler();

			inline const char * c_str() const noexcept {
				return handler_name;
			}

			inline const char * name() const noexcept {
				return handler_name;
			}

			/// @brief Retrieves the schema definition for the interface inputs.
			/// @param[out] schema Object populated with the interface input schema details.
			/// @return True if the interface defines an input schema; false otherwise (schema remains unmodified).
			virtual bool input_schema(Schema &schema) const noexcept;

			/// @brief Retrieves the schema definition for the interface outputs.
			/// @param[out] schema Object populated with the interface output schema details.
			/// @return True if the interface defines an output schema; false otherwise (schema remains unmodified).
			virtual bool output_schema(Schema &schema) const noexcept;

#if __cplusplus >= 202002L
			inline auto operator <=>(const char *name) const noexcept {
				return strcasecmp(name,this->_name);
			}
#else
			inline bool operator==(const char *name) const noexcept {
				return strcasecmp(name,handler_name) == 0;
			}
#endif

			/// @brief Call handler actions.
			/// @param request The request data.
			/// @param response The response data.
			/// @return The return code of the first action to fail.
			/// @retval Complete without failures.
			virtual int call(Udjat::Request &request, Udjat::Response &response) const;

			virtual void push_back(const Properties &props);
			virtual void push_back(std::shared_ptr<Action> action);

		private:
			const char *handler_name;
			std::vector<std::shared_ptr<Action>> actions;

		};

		class UDJAT_API Factory {
		private:
			const char * factory_name;
			const char * factory_description;

		public:
			Factory(const char *name,const char *description = "");
			virtual ~Factory();

			inline const char *name() const noexcept {
				return factory_name;
			}

			inline const char *description() const noexcept {
				return factory_description;
			}

#if __cplusplus >= 202002L
			inline auto operator <=>(const char *name) const noexcept {
				return strcasecmp(name,factory_name);
			}
#else
			inline bool operator==(const char *name) const noexcept {
				return strcasecmp(name,factory_name) == 0;
			}
#endif

			static void build(const Properties &props) noexcept;

			static bool for_each(const std::function<bool(Interface::Factory &intf)> &method);

			virtual void get_properties(Udjat::Value &value) const;

			virtual Interface & InterfaceFactory(const Properties &props) = 0;

		};

		inline const char * name() const noexcept {
			return interface_name;
		}

		inline const char * c_str() const noexcept {
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
		bool allow(const Authentication::Role role = Authentication::None) const;

		/// @brief Call handler actions.
		/// @param request The request data.
		/// @param response The response data.
		/// @return The return code of the first action to fail.
		/// @retval 0 if complete without failures.
		/// @retval ENOENT Request not found.
		/// @retval EPERM Access denied.
		/// @retval EINVAL Invalid arguments on request. 
		/// @retval ENOTSUP if the request is not supported.
		virtual int call(Udjat::Request &request, Udjat::Response &response) const;

		/// @brief Insert interface handler.
		/// @param node The handler description.
		virtual Handler & push_back(const Properties &props);

		virtual ~Interface();

	private:
		const char *interface_name;
		Authentication::Role role = Authentication::None;

	protected:

		typedef Interface Super;

		constexpr Interface(const char *name) : interface_name{name} {
		}

		/// @brief Build an interface from properties.
		/// @param props The properties.
		Interface(const Properties &props);

		/// @brief Push back single action handler.
		/// @param action The action to push back.
		virtual bool push_back(const Properties &props, std::shared_ptr<Action> action);

	};

 }
