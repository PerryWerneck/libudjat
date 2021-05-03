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

 #include <udjat/tools/string.h>
 #include <cstring>
 #include <ctype.h>

 namespace Udjat {

 	char * chomp(char *str) noexcept {

		size_t len = strlen(str);

		while(len--) {

			if(isspace(str[len])) {
				str[len] = 0;
			} else {
				break;
			}
		}

		return str;

	}

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
	char * chug (char *str) noexcept {

		char *start;

		for (start = (char*) str; *start && isspace(*start); start++);

		memmove(str, start, strlen ((char *) start) + 1);

		return str;
	}

	char * strip(char *str) noexcept {
		return chomp(chug(str));
	}

	std::string & strip(std::string &str) noexcept {
		char *buffer = new char[str.size()+1];
		memcpy(buffer,str.c_str(),str.size());
		buffer[str.size()] = 0;
		strip(buffer);
		str.assign(buffer);
		delete[] buffer;
		return str;
	}

	std::string UDJAT_API strip(const char *str, ssize_t length) {

		if(length < 0) {
			length = strlen(str);
		}

		char *buffer = new char[length+1];
		memcpy(buffer,str,length);
		buffer[length] = 0;
		strip(buffer);

		std::string rc(buffer);
		delete[] buffer;

		return rc;
	}


 }

