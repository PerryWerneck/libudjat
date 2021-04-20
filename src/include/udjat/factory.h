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
		Quark name;

	protected:

		/// @brief Factory module info.
		const ModuleInfo *info;

	public:
		Factory(const Quark &name);
		virtual ~Factory();

		static void getInfo(Response &response);

		static bool parse(const char *name, Abstract::Agent &parent, const pugi::xml_node &node);
		static bool parse(const char *name, Abstract::State &parent, const pugi::xml_node &node);

		/// @brief Create Agent child.
		virtual void parse(Abstract::Agent &parent, const pugi::xml_node &node) const;

		/// @brief Create State child.
		virtual void parse(Abstract::State &parent, const pugi::xml_node &node) const;

	};

 }

