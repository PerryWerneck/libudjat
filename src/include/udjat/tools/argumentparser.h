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

#include <udjat/defs.h>
#include <string>
#include <iostream>
#include <functional>
#include <list>
#include <cstring>
#include <cstdint>
#include <udjat/tools/logger.h>

namespace Udjat {

	class UDJAT_API ArgumentParser {
	public:

		/// @brief Return codes for argument parser, every non-zero return will stop parsing.
		enum Result : uint8_t {
			NotHandled		= 0x00,		///< @brief The optional argument was not handled.
			Handled			= 0x01,		///< @brief The optional argument was handled and should be ignored by caller.
			ExitAfterParse	= 0x02,		///< @brief Exit with rc=0 after parsing all options.
			ExitNow			= 0x04,		///< @brief Exit now with rc = 0.
			NotFound 		= 0x08		///< @brief Argument not found.
		};

		enum Flag : uint8_t {
			None 				= 0x00,
			AllowInteractive	= 0x01
		};

		enum Mode : char {
			Undefined			= '\0',	///< @brief No special mode.
			LongOption			= 'L',	///< @brief Parsing a long option.
			ShortOption 		= 'S',	///< @brief Parsing a short option.
			FileArgument		= 'F',	///< @brief Parsing a file/text argument
		};

		class Argument {
		private:

			friend class ArgumentParser;

			const char shortname = 0;			///< @brief Short name of the option.
			const char *longname = nullptr;		///< @brief Long name of the option.
			const char *help = nullptr;			///< @brief Description of the option
			const char *example = nullptr;		///< @brief Example of the option.
			const std::function<Result(const char *argument, const Mode mode)> call = nullptr;
			const Flag flags = None;

		public:

			/// Build parser for non option.
			Argument(const std::function<Result(const char *argument, const Mode mode)> &c) :
				call{c}, flags{None} {
			}

			Argument(Flag f, char s, const char *l, const char *h, const std::function<Result(const char *argument, const Mode mode)> &c) :
				shortname{s}, longname{l}, help{h}, call{c}, flags{f} {
			}

			Argument(Flag f, const char *l, const char *h, const std::function<Result(const char *argument, const Mode mode)> &c) :
				longname{l}, help{h}, call{c}, flags{f} {
			}

			Argument(char s, const char *l, const char *h, const std::function<Result(const char *argument, const Mode mode)> &c) :
				shortname{s}, longname{l}, help{h}, call{c} {
			}

			Argument(char s, const char *l, const char *h, const char *e, const std::function<Result(const char *argument, const Mode mode)> &c) :
				shortname{s}, longname{l}, help{h}, example{e}, call{c} {
			}

			Argument(const char *l, const char *h, const std::function<Result(const char *argument, const Mode mode)> &c) :
				longname{l}, help{h}, call{c} {
			}

			Argument() { 
			}

			inline const char short_option() const noexcept {
				return shortname;
			}

			inline bool operator ==(const char value) const noexcept {
				return shortname && shortname == value;
			}

			inline bool operator ==(const char *value) const noexcept {
				return longname && *longname && strcmp(longname,value) == 0;
			}

			inline operator bool() const noexcept {
				return (bool) (shortname || longname);
			}

			inline Result exec(const char *argument = nullptr, const Mode mode = Undefined) const {
				return call(argument,mode);
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
		ArgumentParser(const char *str);
		ArgumentParser(const Argument &arg);
		~ArgumentParser();

		template<typename... Targs>
		ArgumentParser(const char *str, Targs... Fargs) : ArgumentParser{str} {
			ArgumentParser::append(Fargs...);
		}

		template<typename... Targs>
		ArgumentParser(const Argument &argument, Targs... Fargs) : ArgumentParser{} {
			ArgumentParser::append(argument, Fargs...);
		}

		template<typename... Targs>
		inline void append(const Argument &argument, Targs... Fargs) {
			append(argument);
			append(Fargs...);
		}

		template<typename... Targs>
		inline void append(const char *str, Targs... Fargs) {
			groups.emplace_back(str);
			append(Fargs...);
		}

		inline void append(const Argument &argument) {
			groups.back().push_back(argument);
		}
		
		/// @brief Add argument in the application group.
		/// @param argument The argument to add.
		void add_application_argument(const Argument &argument);

		/// @brief Add argument into the application group.
		/// @param shortname The argument short name.
		/// @param longname The argument long name.
		/// @param help The help text.
		/// @param call Callback to process this argument.
		void add_application_argument(const char shortname, const char *longname, const char *help, const std::function<Result(const char *argument, const char mode)> &call);
		
		/// @brief Add argument in the last group.
		/// @param argument 
		// void append(const Argument &argument);

		/// @brief Add group.
		/// @param text The group title.
		/// @return The new group.
		inline Group & add_group(const std::string &text) {
			return add_group(text.c_str());
		}

		/// @brief Add group.
		/// @param text The group title.
		/// @return The new group.
		Group & add_group(const char *text);

		/// @brief Convenience method to parse a single argument list.
		/// @brief arguments The null terminated argument list to parse.
		/// @throw std::exception on failure.
		/// @return The parse result.
		/// @retval false All the callback have returned false, the application can continue.
		/// @retval true Some callback have returned true, the application should stop with rc = 0.
		static bool parse(int argc, char **argv, const Argument *arguments, const char *help = nullptr);

		/// @brief Parse arguments.
		/// @throw std::exception on failure.
		/// @return Status of the argument parser.
		/// @retval false if the application can continue
		/// @retval true all required processing was done, the application could exit with rc=0.
		bool parse(int argc, char **argv, const char *help = nullptr);

		/// @brief Add options from Logger subsystem in a separate group options.
		/// @return The same object (for chaining).
		ArgumentParser & add_logger_group();

		/// @brief Run option.
		/// @param option The name of the option to run.
		/// @retval NotFound The option was not found.
		Result call(const char *option);

	private:

		struct Context;
		const char *help = nullptr;

		/// @brief The optional argument groups.
		std::list<Group> groups;

		void append_help();

		bool show_help() const;

	};

}


