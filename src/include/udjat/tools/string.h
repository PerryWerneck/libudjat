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
 #include <cstring>
 #include <iostream>
 #include <vector>
 #include <functional>
 #include <udjat/tools/xml.h>

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

	UDJAT_API bool isnumber(const char *str);

	/// @brief Simple markup expander.
	/// @param text String to expand.
	/// @return String with markup expanded.
	UDJAT_API std::string markup(const char *text);

	/// @brief Extended string.
	class UDJAT_API String : public std::string {
	protected:
		virtual void add(const char *str);

	public:
		//
		// Construct
		//
		inline String() : std::string() {
		}

		inline String(const char *str) : std::string{str} {
		}

		inline String(char *str) : std::string{str} {
		}

		inline String(const char *str, size_t length) : std::string(str,length) {
		}

		inline String(const std::string &str) : std::string{str} {
		}

		inline String(std::string &str) : std::string{str} {
		}

		inline String(const Udjat::String &str) : std::string{str} {
		}

		inline String(Udjat::String &str) : std::string{str} {
		}

		template<typename T>
		inline String(const T &str) : std::string{std::to_string(str)} {
		}

		template<typename... Targs>
		String(const char *str, Targs... Fargs) : std::string{str} {
			append(Fargs...);
		}

		template<typename... Targs>
		String(const std::string &str, Targs... Fargs) : std::string{str} {
			append(Fargs...);
		}

		template<typename T, typename... Targs>
		String(const T &str, Targs... Fargs) : std::string{std::to_string(str)} {
			append(Fargs...);
		}

		/// @brief Construct string from xml definition.
		/// @param node XML node with string definitions.
		/// @param attrname XML attribute name for the string value.
		String(const XML::Node &node, const char *attrname = "value", const char *def = nullptr, bool upsearch = true);

		//
		// Append
		//
		virtual void append(const char *str);

		inline void append(char *str) {
			append((const char *) str);
		}

		inline void append(const std::string &str) {
			append(str.c_str());
		}

		inline void append(std::string &str) {
			append(str.c_str());
		}

		inline void append(const Udjat::String &str) {
			append(str.c_str());
		}

		inline void append(Udjat::String &str) {
			append(str.c_str());
		}

		inline void append(const std::exception &e) {
			append(e.what());
		}

		void append(const bool value);

		inline void append() {
		}

		template<typename T>
		inline void append(const T &str) {
			append(std::to_string(str));
		}

		template<typename... Targs>
		void append(const char *str, Targs... Fargs) {
			append(str);
			append(Fargs...);
		}

		template<typename... Targs>
		void append(const std::string &str, Targs... Fargs) {
			append(str.c_str());
			append(Fargs...);
		}

		template<typename... Targs>
		void append(const bool value, Targs... Fargs) {
			append(value);
			append(Fargs...);
		}

		template<typename T, typename... Targs>
		void append(const T str, Targs... Fargs) {
			append(str);
			append(Fargs...);
		}

		//
		// Assign
		//
		inline String & operator = (const char *value) {
			assign(value);
			return *this;
		}

		inline String & operator = (char *value) {
			assign(value);
			return *this;
		}

		inline String & operator = (const std::string &value) {
			assign(value);
			return *this;
		}

		inline String & operator = (std::string &value) {
			assign(value);
			return *this;
		}

		inline String & operator = (const String &value) {
			assign(value);
			return *this;
		}

		inline String & operator = (String &value) {
			assign(value);
			return *this;
		}

		template <typename T>
		inline String & operator = (const T value) {
			return assign(std::to_string(value));
		}

		//
		// Others.
		//
		inline bool operator ==(const char * str) const noexcept {
			return strcasecmp(c_str(),str) == 0;
		}

		inline bool operator ==(const std::string & str) const noexcept {
			return strcasecmp(c_str(),str.c_str()) == 0;
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

		String & markup();

		/// @brief Find first occurrence of substring (case insensitive);
		/// @return Pointer to first occurrence or NULL if not found.
		static char * strcasestr(const char *haystack, const char *needle);

		/// @brief Find first occurrence of substring (case insensitive);
		/// @return Pointer to first occurrence or NULL if not found.
		inline char * strcasestr(const char *needle) {
			return strcasestr(c_str(),needle);
		}

		std::vector<String> split(const char *delim);

		/// @brief Remove the leading whitespace from the string.
		///
		/// Removes leading white spaces from a string, by moving the rest
		/// of the characters forward.
		///
		/// @see chomp() and strip().
		///
		/// @return The string.
		String & chug() noexcept;

		/// @brief Removes trailing white spaces from a string.
		///
		/// @see chug() and strip().
		///
		/// @return The string.
		String & chomp() noexcept;

		bool as_bool(bool def = false);

		/// @brief Convert string to unsigned long long processing 'kb','gb,tb, etc.
		/// @return Unsigned long long value of string contents.
		unsigned long long as_ull() const;

		/// @brief Set byte value, add 'kb', 'gb', 'tb'.
		String & set_byte(unsigned long long value, int precision = 1);

		/// @brief Set byte value, add 'kb', 'gb', 'tb'.
		String & set_byte(double value, int precision = 1);

		/// @brief Return 'quark' string from value.
		const char * as_quark() const;

	};

 }

