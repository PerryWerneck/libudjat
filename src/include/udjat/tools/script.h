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
  * @brief Declares script helper.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

	class UDJAT_API Script : public NamedObject {
	private:
		const char *cmdline = "";
		Logger::Level out = Logger::Info;
		Logger::Level err = Logger::Error;

		int uid = -1;
		int gid = -1;
		bool shell = false;

	protected:

		int run(const char *cmdline);

	public:

		constexpr Script(const char *str) : NamedObject{"scipt"}, cmdline{str} {
		}

		Script(const XML::Node &node);
		~Script();

		/// @brief Run script in foreground.
		/// @return Script return code.
		int run(const Udjat::NamedObject &object);

	};

 }
