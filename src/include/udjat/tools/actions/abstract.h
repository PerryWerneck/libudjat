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

 /**
  * @brief Declares abstract action.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/activatable.h>
 #include <functional>
 #include <list>
 #include <vector>

 namespace Udjat {

	class UDJAT_API Action : public Activatable {
	private:
		class Controller;
		
	protected:
		const char *title;

	public:

		/// @brief The action factory.
		class Factory {
		private:
			const char *name;

		public:
			Factory(const char *name);
			virtual ~Factory();

			inline bool operator==(const char *n) const noexcept {
				return strcasecmp(n,name) == 0;
			}

			inline const char *c_str() const noexcept {
				return name;
			}

			/// @brief Build an action from XML node.
			/// @param node XML definition for the new action.
			virtual std::shared_ptr<Action> ActionFactory(const XML::Node &node) const = 0;

			/// @brief Try to build an action from XML definition.
			/// @param node Action definition.
			/// @return Pointer to new action.
			static std::shared_ptr<Action> build(const XML::Node &node);

			static const std::list<Action::Factory *>::const_iterator begin();
			static const std::list<Action::Factory *>::const_iterator end();

			static bool for_each(const std::function<bool(Action::Factory &factory)> &func) noexcept;

		};

		constexpr Action(const char *n, const char *t = "") : Activatable{n}, title{t} {
		} 

		Action(const XML::Node &node);
		virtual ~Action();

		/// @brief Get action instrospection.
		/// @param call Callback to receive instrospection data.
		virtual void introspect(const std::function<void(const char *name, const Value::Type type, bool in)> &call) const;

		/// @brief Execute action.
		/// @param request The client request.
		/// @param response The response to client.
		/// @param except If false will not launch exception, just return an error code.
		/// @return The return code.
		/// @retval 0 Success.
		virtual int call(Udjat::Request &request, Udjat::Response &response, bool except = true) = 0;

		/// @brief Run action from XML node, usually called by XML tag <init type=>
		virtual int call(bool except = true);

		bool activate(const Udjat::Abstract::Object &object) noexcept override;
		bool activate() noexcept override;

	};

 }
