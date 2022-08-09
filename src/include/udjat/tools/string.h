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
 #include <vector>
 #include <functional>
 #include <udjat/tools/object.h>

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

	/**
	 * @brief String with 'extras'
	 *
	 */
	class UDJAT_API String : public std::string {
	public:
		String() : std::string() {
		}

		String(const char *str) : std::string(str) {
		}

		String(const char *str, size_t length) : std::string(str,length) {
		}

		String(const std::string &str) : std::string(str) {
		}

		/// @brief Test if the string contains one of the elements of a list.
		/// @return Index of the matched content (-1 if not found).
		size_t select(const char *value, ...) __attribute__ ((sentinel));

		/// @brief Insert global expander.
		/// @param method String expander method (returns 'true' if the value was parsed).
		static void push_back(const std::function<bool(const char *key, std::string &value, bool dynamic, bool cleanup)> &method);

		/// @brief Expand ${} macros.
		/// @param expander value expander method.
		/// @param dynamic if true expands the dynamic values like ${timestamp(format)}.
		/// @param cleanup if true remove the non existent values from string.
		String & expand(const std::function<bool(const char *key, std::string &value)> &expander, bool dynamic = false, bool cleanup = false);

		/// @brief Expand ${} macros.
		String & expand(bool dynamic = true, bool cleanup = true);

		/// @brief Expand ${} macros.
		String & expand(const Udjat::Abstract::Object &object, bool dynamic = false, bool cleanup = false);

		/// @brief Expand ${} macros.
		/// @param node XML node from the begin of the value search.
		/// @param group Group from configuration file to search.
		String & expand(const pugi::xml_node &node,const char *group = "default-attributes");

		String & strip() noexcept;

		std::vector<String> split(const char *delim);

		/**
		 * @brief Remove the leading whitespace from the string.
		 *
		 * Removes leading whitespace from a string, by moving the rest
		 * of the characters forward.
		 *
		 * @see chomp() and strip().
		 *
		 * @return The string.
		 *
		 */
		String & chug() noexcept;

		/**
		 * @brief Removes trailing whitespace from a string.
		 *
		 * @see chug() and strip().
		 *
		 * @return The string.
		 *
		 */
		String & chomp() noexcept;

	};

 }

