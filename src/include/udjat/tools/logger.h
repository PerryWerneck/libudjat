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
	class UDJAT_API Logger {
	public:
		enum Level : uint8_t {
			Info,
			Warning,
			Error,
			Trace
		};

	private:
		class Controller;
		friend class Controller;

		class Buffer : public std::string {
		public:
			pthread_t thread;
			Level level;
			Buffer(pthread_t t, Level l) : thread(t), level(l) {
			}

			~Buffer();

			Buffer(const Buffer &src) = delete;
			Buffer(const Buffer *src) = delete;

			bool push_back(int c);

		};

		static std::mutex guard;

		class Writer : public std::basic_streambuf<char, std::char_traits<char> > {
		private:

			/// @brief The buffer id.
			Level id = Info;

			/// @brief Send output to console?
			bool console = true;

#ifndef _WIN32
			void write(int fd, const std::string &str);
#endif // !WIN32

			void write(Buffer &buffer);

		protected:

			/// @brief Writes characters to the associated file from the put area
			int sync() override;

			/// @brief Writes characters to the associated output sequence from the put area.
			int overflow(int c) override;

		public:
			Writer(Logger::Level i, bool c) : id(i), console(c) {
			}

		};

	protected:

		static Level level;

		struct Properties {
			const char * name;	///< @brief Object name.

			constexpr Properties(const char *n) : name(n) {
			}

		} properties;

	public:

		static Level LevelFactory(const char *name) noexcept;

		/// @brief Unformatted Log message.
		class UDJAT_API String : public std::string {
		public:
			template<typename T, typename... Targs>
			String(T &value, Targs... Fargs) {
				append(value);
				add(Fargs...);
			}

			String & append(const char *value) {
				if(!empty()) {
					std::string::append(" ");
				}
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

	public:

		/// @brief Redirect std::cout, std::clog and std::cerr to log file.
		/// @param console If true send log output to standard out.
#ifdef DEBUG
		static void redirect(bool console = true);
#else
		static void redirect(bool console = false);
#endif // DEBUG

		constexpr Logger(const char *name = STRINGIZE_VALUE_OF(PRODUCT_NAME)) : properties(name) {
		}

		void set(const pugi::xml_node &name);

		operator const char *() const noexcept {
			return this->properties.name;
		}

		/// @brief Get Logger name.
		const char * name() const noexcept {
			return this->properties.name;
		}

		inline const char * c_str() const noexcept {
			return this->properties.name;
		}

		/// @brief Write informational message.
		/// @param format String with '{}' placeholders
		template<typename... Targs>
		void info(const char *format, const Targs... args) const noexcept {
			std::cout << name() << "\t" << Message(format, args...) << std::endl;
		}

		/// @brief Write warning message.
		/// @param format String with '{}' placeholders
		template<typename... Targs>
		void warning(const char *format, const Targs... args) const noexcept {
			std::clog << name() << "\t" << Message(format, args...) << std::endl;
		}

		/// @brief Write error message.
		/// @param format String with '{}' placeholders
		template<typename... Targs>
		void error(const char *format, const Targs... args) const noexcept {
			std::cerr << name() << "\t" << Message(format, args...) << std::endl;
		}

		/// @brief Write Error message.
		/// @param e exception to write.
		void error(const std::exception &e) {
			std::cerr << name() << "\t" << e.what() << std::endl;
		}

		/// @brief Write error message.
		/// @param e exception to write.
		/// @param format String with '{}' placeholders (the last one with be the exception message)
		template<typename... Targs>
		void error(const std::exception &e, const char *format, const Targs... args) const noexcept {
			std::cerr << name() << "\t" << Message(format, args...).append(e) << std::endl;
		}

		/// @brief Write message.
		/// @param level	Log level.
		/// @param message	The message.
		static void write(const Level level, const char *message) noexcept;

		/// @brief Write message.
		/// @param level	Log level.
		/// @param message	The message.
		static void write(const Level level, const std::string &message) noexcept;

	};

	#if defined(DEBUG) || defined(TRACE_ENABLED)
		#define trace( ... ) Logger::write(Logger::Trace,Logger::String("trace\t",__VA_ARGS__))
	#else
		#define trace( ... )           // __VA_ARGS__
	#endif // DEBUG

 }

 namespace std {

	inline string to_string(const Udjat::Logger &logger) {
		return logger.name();
	}

	inline string to_string(const Udjat::Logger *logger) {
		return logger->name();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Logger &logger) {
		return os << logger.name();
	}

 }
