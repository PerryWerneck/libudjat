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
 #include <memory>

 namespace Udjat {

	class UDJAT_PRIVATE Updater : private Application {
	private:
		bool changed = false;
		time_t next = 0;
		Application::DataFile path;

	public:
		Updater(const char *pathname);

		void for_each(const std::function<void(const char *filename, const pugi::xml_document &document)> &call);

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

 }