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
 #include <udjat/tools/action.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

	class UDJAT_API Script : public Action {
	private:
		const char *cmdline = "";
		const char *title = "";

#ifndef _WIN32
		int uid = -1;
		int gid = -1;
		bool shell = false;
#endif // !_WIN32

	protected:

		Logger::Level out = Logger::Info;
		Logger::Level err = Logger::Error;

		int run(const char *cmdline) const;

	public:

		constexpr Script(const char *str, const char *name = "script") : Action{name}, cmdline{str} {
		}

		Script(const XML::Node &node, const char *title = "");
		~Script();

		/// @brief Run script in foreground.
		/// @param request The values for cmdline expansion and store return code.
		/// @param response The value to receive the return code.
		/// @param except If true the action will launch exception on failure.
		/// @return The return code.
		/// @retval 0 Success.
		int call(Udjat::Request &request, Udjat::Response &response, bool except = false) override;

		/// @brief Run script in foreground.
		/// @param object The object for cmdline expansion.
		/// @param except If true the action will launch exception on failure.
		/// @return The return code.
		/// @retval 0 Success.
		int run(const Udjat::NamedObject &object, bool except = true) const;

		/// @brief Get string title.
		const char *c_str() const noexcept;

		/// @brief Get script title.
		std::string to_string() const noexcept;

	};

 }
