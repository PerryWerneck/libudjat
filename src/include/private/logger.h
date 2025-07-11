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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <pugixml.hpp>
 #include <list>
 #include <mutex>

 #ifdef DEBUG
	#define DEBUG_ENABLED true
 #else
	#define DEBUG_ENABLED false
 #endif // DEBUG

 namespace Udjat {

	namespace Logger {

#ifndef _WIN32
		bool write(int fd, const char *text);
		void timestamp(int fd);
#endif // !WIN32

		UDJAT_PRIVATE void setup(const XML::Node &node) noexcept;

		UDJAT_PRIVATE void dummy_writer(Level level, const char *domain, const char *text) noexcept;
		UDJAT_PRIVATE void file_writer(Level level, const char *domain, const char *text) noexcept;
		UDJAT_PRIVATE void console_writer(Level level, const char *domain, const char *text) noexcept;

#ifndef _WIN32
		UDJAT_PRIVATE const char * decoration(Level level) noexcept;
#endif // _WIN32

		struct UDJAT_PRIVATE Options {

			/// @brief Console writer.
			void (*console)(Level level, const char *domain, const char *text) = console_writer;

			/// @brief File writer (disabled by default).
			void (*file)(Level level, const char *domain, const char *text) = nullptr;

			/// @brief Custom log file name.
			const char *filename = nullptr;

#ifndef _WIN32
			bool syslog = true;
#endif // !_WIN32

			bool enabled[Logger::Debug+2] = {
				true,				// Informational message.
				true,				// Warning conditions.
				true,				// Error conditions.
				DEBUG_ENABLED,		// Trace message.

				// Allways the last ones.
				DEBUG_ENABLED,		// Debug message.
				true,				// Notify message.
			};

			static Options & getInstance();

		};

		class UDJAT_PRIVATE Buffer : public std::string {
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

		class UDJAT_PRIVATE Writer : public std::basic_streambuf<char, std::char_traits<char> > {
		private:

			/// @brief The Log level.
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
			Writer(Logger::Level i) : id(i) {
			}

		};

		class UDJAT_PRIVATE Controller {
		private:
			std::mutex guard;
			std::list<Buffer *> buffers;

		public:

			Controller(const Controller &src) = delete;
			Controller(const Controller *src) = delete;

			Controller();

			~Controller();

			Buffer * BufferFactory(Level id);
			void remove(Buffer *buffer) noexcept;

			static Controller & getInstance();


		};

	}

 }
