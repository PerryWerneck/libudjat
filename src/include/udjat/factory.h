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

 #include <udjat/defs.h>
 #include <udjat/request.h>

 namespace Udjat {

	class UDJAT_API Factory {
	private:
		class Controller;
		const char *name = "";

	protected:

		/// @brief Factory module info.
		const ModuleInfo &info;

		/// @brief Types.
		struct {
			/// @brief Agent type (to build agents from the 'type=' attribute on XML).
			const char *agent = nullptr;

			/// @brief Alert type (to build alerts from the 'type=' attribute on XML).
			const char *type = nullptr;

		} type;

	public:
		Factory(const char *name);
		Factory(const char *name, const ModuleInfo &info);
		Factory(const Quark &name, const ModuleInfo &info) : Factory(name.c_str(),info) {
		}

		virtual ~Factory();

		inline const char * getName() const {
			return name;
		}

		static void getInfo(Response &response);

		static bool parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node);
		static bool parse(const char *name, Abstract::State &parent, const pugi::xml_node &node);

		/// @brief Create Agent child.
		/// @param parent Parent agent to insert the built child.
		/// @param node XML definition for the new agent.
		/// @return true if the request was handled.
		virtual bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const;

		/// @brief Create State child.
		/// @param parent Parent state insert the built child.
		/// @param node XML definition for the new state.
		/// @return true if the request was handled.
		virtual bool parse(Abstract::State &parent, const pugi::xml_node &node) const;


	};

 }

