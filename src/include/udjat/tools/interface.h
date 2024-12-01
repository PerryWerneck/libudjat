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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/action.h>
 #include <vector>
 #include <memory>
 #include <vector>

 namespace Udjat {

	/// @brief Abstract handler.
	class UDJAT_API Interface {
	private:
		const char *_name;

	protected:

		typedef Interface Super;

		/// @brief Build an interface from XML description
		/// @param name The interface declaration.
		Interface(const XML::Node &node);

		inline const char * c_str() const noexcept {
			return _name;
		}

#if __cplusplus >= 202002L
		inline auto operator <=>(const char *name) const noexcept {
			return strcasecmp(name,this->_name);
		}
#else
		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(name,this->_name) == 0;
		}
#endif

		/// @brief A request handler method.
		class UDJAT_API Handler {
		public:

			/// @brief Interface method introspection.
			struct Introspection {
				/// @brief The value direction.
				enum Direction : uint8_t {
					Both,
					Input,
					Output
				} direction;
				Value::Type type;	///< @brief The type value.
				const char *name;	///< @brief The argument name.
				Introspection(const XML::Node &node);
			};

			Handler(const XML::Node &node);
			Handler(const char *name, const XML::Node &node);
			virtual ~Handler();

			inline const char * c_str() const noexcept {
				return _name;
			}

			bool for_each(const std::function<bool(const Introspection &instrospection)> &call) const;

#if __cplusplus >= 202002L
			inline auto operator <=>(const char *name) const noexcept {
				return strcasecmp(name,this->_name);
			}
#else
			inline bool operator==(const char *name) const noexcept {
				return strcasecmp(name,this->_name) == 0;
			}
#endif

			/// @brief Clear objects, build children.
			/// @param request The request for this interface.
			/// @param response The response for this interface.
			void clear(Udjat::Value &request, Udjat::Value &response) const;

			/// @brief Call handler actions.
			/// @param request The request data.
			/// @param response The response data.
			/// @return The return code of the first action to fail.
			/// @retval Complete without failures.
			int call(Udjat::Request &request, Udjat::Response &response) const;

		private:
			const char *_name;
			std::vector<Introspection> introspection;
			std::vector<std::shared_ptr<Action>> actions;

			/// @brief Create children if not exists.
			/// @param request The request for this interface.
			/// @param response The response for this interface.
			void prepare(Udjat::Value &request, Udjat::Value &response) const;

		};

	public:

		class UDJAT_API Factory {
		private:
			const char *_name;

		public:
			Factory(const char *name);
			virtual ~Factory();

#if __cplusplus >= 202002L
			inline auto operator <=>(const char *_name) const noexcept {
				return strcasecmp(name,this->_name);
			}
#else
			inline bool operator==(const char *name) const noexcept {
				return strcasecmp(name,this->_name) == 0;
			}
#endif

			static void build(const XML::Node &node) noexcept;

			static bool for_each(const std::function<bool(Interface::Factory &interface)> &method);

			virtual Interface & InterfaceFactory(const XML::Node &node) = 0;

		};

		virtual ~Interface();

	};

 }
