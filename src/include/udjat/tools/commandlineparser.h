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
#include <iostream>

namespace Udjat {

	/// @brief Parse command line arguments.
	class UDJAT_API CommandLineParser {
	private:
		int &argc;
		char **argv;

	public:

		/// @brief Command line argument.
		/// @details This structure is used to define command line options for 'help' output.
		struct Argument {

			const char shortname;		///< @brief Short name of the option.
			const char *longname;		///< @brief Long name of the option.
			const char *description;	///< @brief Description of the option.

			constexpr Argument(char s, const char *l, const char *d) :
				shortname{s}, longname{l}, description{d} {
			}

			constexpr Argument() :
				shortname{0}, longname{nullptr}, description{nullptr} {
			}

			std::ostream & print(std::ostream &out, size_t width = 20) const;

		};

		constexpr CommandLineParser(int c, char **v) : argc(c), argv(v) {
		}

		inline const char *appname() const noexcept {
			return argv[0];
		}

		static bool has_argument(int &argc, char **argv, char shortname, const char *longname, bool extract=true) noexcept;

		static bool get_argument(int &argc, char **argv, char shortname, const char *longname, std::string &value, bool extract=true) noexcept;

		/// @brief Check standard command line options.
		/// @param argc Number of command line arguments.
		/// @param argv The command line arguments.
		/// @param options Argument definitions for command line options.
		/// @param dbg true if debug mode is enabled, false otherwise.
		/// @param width Width of the left part of the help text.
		/// @return false if the '--help' option was found, processed and the application should not run, true otherwise.
#ifdef DEBUG 
		static bool options(int &argc, char **argv, const CommandLineParser::Argument *options, bool dbg = true, size_t width = 20) noexcept;
#else
		static bool options(int &argc, char **argv, const CommandLineParser::Argument *options, bool dbg = false, size_t width = 20) noexcept;
#endif

		/// @brief Pop command line argument. 
		/// @details Scan command line options from arguments, if found extract it.
		/// @return true if the argument was found
		inline bool pop(char shortname, const char *longname) {
			return has_argument(argc, argv, shortname, longname, true);
		}

		inline bool pop(char shortname, const char *longname, std::string &value) {
			return get_argument(argc, argv, shortname, longname, value, true);
		}

	};

}

 namespace std {

	inline ostream& operator<< (ostream& os, const Udjat::CommandLineParser::Argument &opt) {
			return opt.print(os);
	}

 }

