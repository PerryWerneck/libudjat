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

 namespace Udjat {

	class UDJAT_API Action : public NamedObject {
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

			/// @brief Create an action from XML node.
			/// @param node XML definition for the new action.
			virtual std::shared_ptr<Action> ActionFactory(const XML::Node &node) const = 0;

			static std::shared_ptr<Action> build(const XML::Node &node);

		};

		Action(const XML::Node &node);
		virtual ~Action();

		/// @brief Execute action
		/// @param value The in/out values.
		virtual void call(Udjat::Value &value) = 0;

		/// @brief Execute action.
		/// @param request The client request.
		/// @param response The response to client.
		virtual void call(Udjat::Request &request, Udjat::Response &response);

	};

 }
