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
 #include <udjat/tools/properties.h>
 #include <udjat/tools/argumentparser.h>
 #include <iostream>
 #include <mutex>
 #include <pthread.h>
 #include <cstdint>

 namespace Udjat {

	namespace Logger {

		enum Level : uint8_t {
			None		= 0x00,		///< @brief No log messages.
			Notice		= 0x01,		///< @brief System Status
			Error		= 0x02,		///< @brief Error conditions (std::cerr).
			Warning		= 0x04,		///< @brief Warning conditions (std::clog).
			Info		= 0x08,		///< @brief Informational message (std::cout).
			Trace		= 0x10,		///< @brief Debug message.
			Debug		= 0x20,		///< @brief Trace message
		};

		Logger::Level UDJAT_API LevelFactory(const Properties &props, const char *attr, const char *def);
		Logger::Level UDJAT_API LevelFactory(const char *name);

		/// @brief Redirect std::cout, std::cerr & std::clog to log system.
		UDJAT_API void redirect();

		UDJAT_API void verbosity(const char *level) noexcept;
		UDJAT_API void verbosity(int level) noexcept;
		UDJAT_API int verbosity() noexcept;

		/// @brief Show help messages.
		/// @param width The width of the left part of the help text.
		/// @details This method is called when the application is started with the '--help' option.
		UDJAT_API void help(size_t width = 20) noexcept;

		/// @brief Test if the log level is enabled.
		/// @param level The log level to test.
		/// @return The current state.
		/// @retval true The level is enabled.
		/// @retval falt The level is disabled.
		UDJAT_API bool enabled(Logger::Level level) noexcept;

		UDJAT_API void enable(Logger::Level level, bool enabled = true) noexcept;

		/// @brief Enable/Disable default console backend.
		/// @return The console state before the change.
		/// @retval true the console was enabled.
		/// @retval false the console was not enabled.
		UDJAT_API bool console(bool enable);

		/// @brief Get state of console backend.
		/// @return The current state.
		/// @retval true The console backend is enabled.
		/// @retval false The console backend is disabled.
		UDJAT_API bool console();

		/// @brief Enable/Disable default file writer.
		/// @param filename The log filename (nullptr to disable file).
		/// @param max_age Max age for the file, in seconds.
		UDJAT_API void file(const char *filename = "%Y-%m-%d.log", time_t max_age = 86400);

		/// @brief Setup logger engine from configuration file.
		/// @param group The group from configuration file with logger engine options.
		UDJAT_API void setup(const char *group = "logger") noexcept;

		/// @brief Setup log options from properties.
		/// @param properties The properties for logger.
		/// @param prefix The optional prefix for the attributes on properties.
#ifdef DEBUG
		UDJAT_API void setup(const Properties &properties, const char *prefix = "", bool dbg = true) noexcept;
#else
		UDJAT_API void setup(const Properties &properties, const char *prefix = "", bool dbg = false) noexcept;
#endif // DEBUG

		/// @brief Setup logger from command line.
		/// @param argc	 The number of arguments. 
		/// @param argv  The command line arguments.
		/// @param dbg True to use debug mode defaults.
#ifdef DEBUG
		UDJAT_API void setup(int &argc, char **argv, bool extract = true, bool dbg = true);
		UDJAT_API void setup(int argc, char **argv, bool dbg = true);
#else
		UDJAT_API void setup(int &argc, char **argv, bool extract = true, bool dbg = false);
		UDJAT_API void setup(int argc, char **argv, bool dbg = false);
#endif // DEBUG

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

			inline void notice(const char *domain = LOG_DOMAIN) const {
				write(Logger::Notice,domain);
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

			inline void notice(const char *domain = PACKAGE_NAME) const {
				write(Logger::Notice,domain);
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

			inline void notice(const char *domain = "") const {
				write(Logger::Notice,domain);
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

			inline void notice(const char *domain = "") const {
				write(Logger::Notice,domain);
			}

		};

		UDJAT_API std::ostream & info();
		UDJAT_API std::ostream & warning();
		UDJAT_API std::ostream & error();
		UDJAT_API std::ostream & trace();

	}

	#if defined(DEBUG)
		#define debug( ... ) Udjat::Logger::String(__FILE__,"(",__LINE__,"): ",__VA_ARGS__).write(Udjat::Logger::Debug,"debug");
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

