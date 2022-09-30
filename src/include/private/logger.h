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
 #include <udjat/tools/logger.h>
 #include <list>

 namespace Udjat {

 	class UDJAT_PRIVATE Logger::Controller {
	public:
		std::list<Buffer *> buffers;

		Controller(const Controller &src) = delete;
		Controller(const Controller *src) = delete;

		Controller();

		~Controller();

		static Controller & getInstance();

		Buffer * BufferFactory(Level id);

		/// @brief Write log message.
		/// @param level The loglevel.
		/// @param console Show it on console?
		/// @param message The log message.
		void write(const Level level, bool console, const char *message) noexcept;

	};

 }