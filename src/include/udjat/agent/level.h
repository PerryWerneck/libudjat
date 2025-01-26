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
 #include <udjat/tools/xml.h>
 #include <cstdint>

 namespace Udjat {

	/// @brief Alert/state level.
	enum Level : uint8_t {
		undefined,
		unimportant,
		ready,
		warning,
		error,

		critical		///< @brief Critical level (always the last one)

	};

	/// @brief Get level from string.
	UDJAT_API Level LevelFactory(const char *name);

	/// @brief Get level from XML node.
	UDJAT_API Level LevelFactory(const XML::Node &node);

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Level level);

	inline ostream& operator<< (ostream& os, const Udjat::Level level) {
		return os << to_string(level);
	}

 }
