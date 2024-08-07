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
 #include <string>
 #include <iostream>


 namespace Udjat {

	/**
	 * @brief Removes trailing whitespace from a string.
	 *
	 * This function doesn't allocate or reallocate any memory;
	 * it modifies in place. Therefore, it cannot be used
	 * on statically allocated strings.
	 *
	 * Reference: <https://git.gnome.org/browse/glib/tree/glib/gstrfuncs.c>
	 *
	 * @see chug() and strip().
	 *
	 * @return pointer to string.
	 *
	 */
	UDJAT_API char * chomp(char *str) noexcept;

	/**
	 * @brief Remove the leading whitespace from the string.
	 *
	 * Removes leading whitespace from a string, by moving the rest
	 * of the characters forward.
	 *
	 * This function doesn't allocate or reallocate any memory;
	 * it modifies the string in place. Therefore, it cannot be used on
	 * statically allocated strings.
	 *
	 * Reference: <https://git.gnome.org/browse/glib/tree/glib/gstrfuncs.c>
	 *
	 * @see chomp() and strip().
	 *
	 * @return pointer to string.
	 *
	 */
	UDJAT_API char * chug(char *str) noexcept;

	UDJAT_API char * strip(char *str) noexcept;

	UDJAT_API std::string & strip(std::string &str) noexcept;

	UDJAT_API std::string strip(const char *str, ssize_t length = -1);

 }

