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
 #include <udjat/tools/string.h>
 #include <udjat/tools/message.h>
 #include <iostream>
 #include <mutex>
 #include <pthread.h>
 #include <pugixml.hpp>
 #include <cstdint>

 namespace Udjat {

	template <typename I> 
	inline std::string n2hexstr(I w, size_t hex_len = sizeof(I)<<1) {
		static const char* digits = "0123456789ABCDEF";
		std::string rc(hex_len,'0');
		for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
			rc[i] = digits[(w>>j) & 0x0f];
		return rc;
	}

	/// @brief Manage log files.
	/// Reference: https://www.youtube.com/watch?v=IdM0Z2a4fjU
	namespace Logger {

		enum Level : uint8_t {
			Error,		///< @brief Error conditions (std::cerr).
			Warning,	///< @brief Warning conditions (std::clog).
			Info,		///< @brief Informational message (std::cout>.
			Trace,		///< @brief Debug message.

			// Debug should be the last one.
			Debug		///< @brief Trace message
		};

		UDJAT_API unsigned short verbosity() noexcept;
		UDJAT_API void verbosity(unsigned short level) noexcept;
		UDJAT_API void verbosity(const char *level);

		UDJAT_API std::ostream & info();
		UDJAT_API std::ostream & warning();
		UDJAT_API std::ostream & error();
		UDJAT_API std::ostream & trace();

		/// @brief Show help messages.
		/// @param width The width of the left part of the help text.
		/// @details This method is called when the application is started with the '--help' option.
		UDJAT_API void help(size_t width = 20) noexcept;

		/// @brief Setup logger from command line.
		/// @param argc	 The number of arguments. 
		/// @param argv  The command line arguments.
		/// @param dbg True to use debug mode defaults.
#ifdef DEBUG
		UDJAT_API void setup(int &argc, char **argv, bool extract = true, bool dbg = true);
#else
		UDJAT_API void setup(int &argc, char **argv, bool extract = true, bool dbg = false);
#endif // DEBUG

		/// @brief Enable/Disable write to file.
		/// @param Log level.
		/// @param enabled true to enable write to file in label.
		UDJAT_API void enable(Level level, bool enabled = true) noexcept;

		/// @brief Check if log level is enabled.
		/// @param Log level.
		/// @return true if the level is enabled.
		UDJAT_API bool enabled(Level level) noexcept;

		/// @brief Write message.
		/// @param level	Log level.
		/// @param message	The message.
		UDJAT_API void write(const Level level, const char *message) noexcept;

		/// @brief Write message.
		/// @param level	Log level.
		/// @param domain	The message domain.
		/// @param message	The message.
		/// @param force	If true ignore level 'enable' option.
		UDJAT_API void write(const Level level, const char *domain, const char *message, bool force) noexcept;

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
		UDJAT_API Level LevelFactory(const XML::Node &node, const char *attr, const char *def);

		/// @brief Unformatted Log message.
		class UDJAT_API String : public Udjat::String {
		public:

			template<typename... Targs>
			inline String(Targs... Fargs) : Udjat::String{Fargs...} {
			}

#if defined(LOG_DOMAIN)

			void write(const Logger::Level level, const char *domain = LOG_DOMAIN) const;

			inline void trace(const char *domain = LOG_DOMAIN) const {
				write(Logger::Trace,domain);
			}

			inline void info(const char *domain = LOG_DOMAIN) const {
				write(Logger::Info,domain);
			}

			inline void warning(const char *domain = LOG_DOMAIN) const {
				write(Logger::Warning,domain);
			}

			inline void error(const char *domain = LOG_DOMAIN) const {
				write(Logger::Error,domain);
			}

#elif defined(PACKAGE_NAME)
			void write(const Logger::Level level, const char *domain = PACKAGE_NAME) const;

			inline void trace(const char *domain = PACKAGE_NAME) const {
				write(Logger::Trace,domain);
			}

			inline void info(const char *domain = PACKAGE_NAME) const {
				write(Logger::Info,domain);
			}

			inline void warning(const char *domain = PACKAGE_NAME) const {
				write(Logger::Warning,domain);
			}

			inline void error(const char *domain = PACKAGE_NAME) const {
				write(Logger::Error,domain);
			}

#else
			void write(const Logger::Level level, const char *domain = "") const;

			inline void trace(const char *domain = "") const {
				write(Logger::Trace,domain);
			}

			inline void info(const char *domain = "") const {
				write(Logger::Info,domain);
			}

			inline void warning(const char *domain = "") const {
				write(Logger::Warning,domain);
			}

			inline void error(const char *domain = "") const {
				write(Logger::Error,domain);
			}

#endif

			inline void trace(const std::string &domain) const {
				write(Logger::Trace,domain.c_str());
			}

			inline void info(const std::string &domain) const {
				write(Logger::Info,domain.c_str());
			}

			inline void warning(const std::string &domain) const {
				write(Logger::Warning,domain.c_str());
			}

			inline void error(const std::string &domain) const {
				write(Logger::Error,domain.c_str());
			}

		};

		/// @brief Formatted Log message.
		class UDJAT_API Message : public Udjat::Message {
		public:

			template<typename... Targs>
			inline Message(const char *fmt, Targs... Fargs) : Udjat::Message{fmt, Fargs...} {
			}

			void write(const Logger::Level level, const char *domain = "") const;

			inline void trace(const char *domain = "") const {
				write(Logger::Trace,domain);
			}

			inline void info(const char *domain = "") const {
				write(Logger::Info,domain);
			}

			inline void warning(const char *domain = "") const {
				write(Logger::Warning,domain);
			}

			inline void error(const char *domain = "") const {
				write(Logger::Error,domain);
			}

		};

		/// @brief Redirect std::cout, std::clog and std::cerr to log file.
		UDJAT_API void redirect();

		/// @brief Enable/Disable default console writer.
		UDJAT_API void console(bool enable);
		UDJAT_API bool console();

		/// @brief Enable/Disable default file writer.
		UDJAT_API bool file();
		UDJAT_API void file(const char *filename);
		UDJAT_API void file(bool enable);

		/// @brief Replace console/file writers.
		UDJAT_API void console(void (*writer)(Level, const char *, const char *));
		UDJAT_API void file(void (*writer)(Level, const char *, const char *));

#ifndef _WIN32
		UDJAT_API void syslog(bool enable);
		UDJAT_API bool syslog();
#endif // _WIN32

	};

	#if defined(DEBUG)
		#define debug( ... ) Udjat::Logger::String(__FILE__,"(",__LINE__,"): ",__VA_ARGS__).write(Logger::Debug,"debug");
	#else
		#define debug( ... )           // __VA_ARGS__
	#endif // DEBUG

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Logger::Level level);

	inline ostream & operator<< (ostream& os, const Udjat::Logger::Level level) {
		return os << to_string(level);
	}

 }

