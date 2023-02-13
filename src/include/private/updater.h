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

 #include <config.h>
 #include <udjat/defs.h>
 #include <iostream>
 #include <udjat/tools/application.h>
 #include <udjat/tools/xml.h>
 #include <memory>
 #include <list>

 namespace Udjat {

 	class UDJAT_PRIVATE Updater : public std::list<std::string> {
	private:
		time_t next = 0;			///< @brief Seconds for next update.
		bool update;				///< @brief true if an update was requested.
		Application::Name name;		///< @brief Application name.

	public:
		Updater(const char *pathname, bool force);

		/// @brief Refresh XML files (if necessary);
		/// @return true to reconfigure.
		bool refresh();

		/// @brief Load configuration files.
		/// @param agent New root agent.
		/// @return True on success.
		bool load(std::shared_ptr<Abstract::Agent> root) const noexcept;

		/// @brief Get seconds for next update.
		inline time_t wait() const noexcept {
			return next;
		}

		/// @brief Write to the 'information' stream.
		inline std::ostream & info() const {
			return Application::info();
		}

		/// @brief Write to the 'warning' stream.
		inline std::ostream & warning() const {
			return Application::warning();
		}

		/// @brief Write to the 'error' stream.
		inline std::ostream & error() const {
			return Application::error();
		}


 	};

 }
