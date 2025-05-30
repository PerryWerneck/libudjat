/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
#include <string>

namespace Udjat {

	/// @brief Parse command line arguments.
	class UDJAT_API CommandLineParser {
	private:
		int &argc;
		char **argv;

	public:
		constexpr CommandLineParser(int c, char **v) : argc(c), argv(v) {
		}

		inline const char *appname() const noexcept {
			return argv[0];
		}

		static bool pop(int &argc, char **argv, char shortname, const char *longname);
		static bool pop(int &argc, char **argv, char shortname, const char *longname, std::string &value);

		/// @brief Pop command line argument. 
		/// @details Scan command line options from arguments, if found extract it.
		/// @return true if the argument was found
		inline bool pop(char shortname, const char *longname) {
			return pop(argc, argv, shortname, longname);
		}

		inline bool pop(char shortname, const char *longname, std::string &value) {
			return pop(argc, argv, shortname, longname, value);
		}


	};


}