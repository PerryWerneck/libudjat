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
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/intl.h>
 #include <udjat/agent.h>
 #include <functional>
 #include <thread>
 #include <list>

 #ifndef HAVE_STRPTIME
	UDJAT_PRIVATE char *strptime(const char *buf, const char *fmt, struct tm *tm);
 #endif // !HAVE_STRPTIME

 namespace Udjat {

	/// @brief Set Root agent.
	/// @param agent Pointer to a valid root agent.
	// UDJAT_PRIVATE void setRootAgent(std::shared_ptr<Abstract::Agent> agent);

	/// @brief Load modules from XML definitions.
	UDJAT_PRIVATE void load_modules(const char *pathname);

	/// @brief Get new definition files from server.
	/// @returns Seconds for the next refresh.
	UDJAT_PRIVATE time_t refresh_definitions(const char *pathname);

	/// @brief Load agent definitions from files.
	/// @param agent Root agent.
	/// @param pathname Path for XML agent definitions.
	UDJAT_PRIVATE void load_agent_definitions(std::shared_ptr<Abstract::Agent> agent,const char *pathname);

 }

