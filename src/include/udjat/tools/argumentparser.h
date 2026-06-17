/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
#include <functional>
#include <list>
#include <cstring>

namespace Udjat {

	class UDJAT_API ArgumentParser {
	public:

		class Argument {
		private:
		
			friend class ArgumentParser;

			const char shortname = 0;			///< @brief Short name of the option.
			const char *longname = nullptr;		///< @brief Long name of the option.
			const char *help = nullptr;			///< @brief Description of the option///< @brief Description of the option.
			const std::function<bool(const char *argument)> &call = nullptr;

		public:
			Argument(char s, const char *l, const char *h, const std::function<bool(const char *argument)> &c) :
				shortname{s}, longname{l}, help{h}, call{c} {
			}

			Argument() { 
			}

			inline bool operator ==(const char value) const noexcept {
				return shortname == value && shortname;
			}

			inline bool operator ==(const char *value) const noexcept {
				return longname && *longname && strcmp(longname,value) == 0;
			}

			inline operator bool() const noexcept {
				return (bool) (shortname || longname);
			}

			inline bool exec(const char *argument = nullptr) const {
				return call(argument);
			}

		};

		class Group : public std::list<Argument>{
		private:
			const char *title;

		public:
			Group(const char *t) : title{t} {
			}

			~Group() {
			}

			inline const char *c_str() const noexcept {
				return title;
			}

			inline operator const char *() const noexcept {
				return title;
			}

		};

		ArgumentParser();
		~ArgumentParser();

		/// @brief Add argument into the application group.
		/// @param argument The argument to add.
		void add_argument(const Argument &argument);

		/// @brief Add argument into the application group.
		/// @param shortname The argument short name.
		/// @param longname The argument long name.
		/// @param help The help text.
		/// @param call Callback to process this argument.
		void add_argument(const char shortname, const char *longname, const char *help, const std::function<bool(const char *argument)> &call);
		
		/// @brief Add group.
		/// @param text The group title.
		/// @return The new group.
		Group & add_group(const char *text);

		/// @brief Convenience method to parse simple argument list.
		/// @brief arguments The null terminated argument list to parse.
		/// @throw std::exception on failure.
		/// @return The parse result.
		/// @retval false All the callback have returned false, the application can continue.
		/// @retval true Some callback have returned true, the application should stop with rc = 0.
		static bool parse(int &argc, const char **argv, const Argument *arguments);

		/// @brief Parse arguments.
		/// @throw std::exception on failure.
		/// @return true if the application can continue, false if it should exit.
		bool parse(int &argc, const char **argv) const;

	private:

		/// @brief The optional argument groups.
		std::list<Group> groups;

		bool show_help() const;
		bool parse_short(const char **argument, const char **argv) const;
		bool parse_long(const char *argument, const char **argv) const;

	};

}


