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
 #include <mutex>
 #include <pthread.h>
 #include <pugixml.hpp>

 namespace Udjat {

	/// @brief Manage log files.
	/// Reference: https://www.youtube.com/watch?v=IdM0Z2a4fjU
	namespace Logger {

		enum Level : uint8_t {
			Info,
			Warning,
			Error,
			Debug,

			Trace	// Trace should be the last one.
		};

		UDJAT_API std::ostream & info();
		UDJAT_API std::ostream & warning();
		UDJAT_API std::ostream & error();
		UDJAT_API std::ostream & trace();

		/// @brief Write message.
		/// @param level	Log level.
		/// @param message	The message.
		UDJAT_API void write(const Level level, const char *message) noexcept;

		/// @brief Write message.
		/// @param level	Log level.
		/// @param domain	The message domain.
		/// @param message	The message.
		UDJAT_API void write(const Level level, const char *domain, const char *message) noexcept;

		/// @brief Write message.
		/// @param level	Log level.
		/// @param message	The message.
		UDJAT_API void write(const Level level, const std::string &message) noexcept;

		UDJAT_API Level LevelFactory(const char *name) noexcept;

		/// @brief Unformatted Log message.
		class UDJAT_API String : public std::string {
		public:
			void write(const Logger::Level level) const;
			void write(const Logger::Level level, const char *domain) const;

			inline void trace(const char *domain) const {
				write(Logger::Trace,domain);
			}

			String() = default;

			String(const char *str) : std::string(str) {
			}

			String(const std::string &str) : std::string(str) {
			};

			template<typename T>
			String(const T &value) : std::string(std::to_string(value)) {
			}

			template<typename T, typename... Targs>
			String(T &value, Targs... Fargs) {
				append(value);
				add(Fargs...);
			}

			String & append(const char *value) {
				std::string::append(value);
				return *this;
			}

			String & append(char *value) {
				std::string::append(value);
				return *this;
			}

			String & append(const std::string &value) {
				return append(value.c_str());
			}

			String & append(const std::exception &e);

			String & add() {
				return *this;
			}

			template<typename T>
			String & append(const T &value) {
				return append(std::to_string(value));
			}

			template<typename T, typename... Targs>
			String & add(T &value, Targs... Fargs) {
				append(value);
				return add(Fargs...);
			}

		};

		/// @brief Formatted Log message.
		class UDJAT_API Message : public std::string {
		public:
			template<typename T, typename... Targs>
			Message(const char *fmt, T &value, Targs... Fargs) : std::string(fmt) {
				append(value);
				add(Fargs...);
			}

			Message & append(const char *value);

			Message & append(const std::string &value) {
				return append(value.c_str());
			}

			Message & append(const std::exception &e);

			Message & add() {
				return *this;
			}

			template<typename T>
			Message & append(const T &value) {
				return append(std::to_string(value));
			}

			template<typename T, typename... Targs>
			Message & add(T &value, Targs... Fargs) {
				append(value);
				return add(Fargs...);
			}

		};

		/// @brief Redirect std::cout, std::clog and std::cerr to log file.
		/// @param console If true send log output to standard out.
#ifdef DEBUG
		UDJAT_API void redirect(bool console = true, bool file = true);
		UDJAT_API void console(bool enable = true);

#else
		UDJAT_API void redirect(bool console = false, bool file = true);
		UDJAT_API void console(bool enable = false);
#endif // DEBUG

		UDJAT_API bool file();
		UDJAT_API bool console();

#ifdef _WIN32
		UDJAT_API void file(bool enable = true);
#else
		UDJAT_API void file(bool enable = false);
		UDJAT_API void syslog(bool enable = true);
		UDJAT_API bool syslog();
#endif // _WIN32

	};

	#if defined(DEBUG)
		#define debug( ... ) Udjat::Logger::String(__FILE__,"(",__LINE__,"): ",__VA_ARGS__).write(Logger::Trace,"debug");
	#else
		#define debug( ... )           // __VA_ARGS__
	#endif // DEBUG

 }
