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

 	class UDJAT_PRIVATE Updater : private Application, public std::list<std::string> {
	private:
		time_t next = 0;	///< @brief Seconds for next update.
		bool update;		///< @brief true if an update was requested.

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

 	};

	/*
	class UDJAT_PRIVATE Updater : private Application {
	private:
		bool changed = false;
		time_t next = 0;
		Application::DataFile path;

	public:
		Updater(const char *pathname);

		inline void for_each(const std::function<void(const char *filename, const pugi::xml_document &document)> &call) {
			Udjat::for_each(path.c_str(),call);
		}

		/// @brief Update agent, set it as a new root.
		time_t set(std::shared_ptr<Abstract::Agent> agent) noexcept;

		inline operator bool() const noexcept {
			return changed;
		}

		inline time_t time() const noexcept {
			return next;
		}

		inline const char * to_string() const noexcept {
			return path.c_str();
		}

	};
	*/

 }
