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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/properties.h>
 #include <mutex>
 #include <functional>
 #include <list>
 #include <string>

 #ifdef DEBUG
	#define DEBUG_ENABLED true
 #else
	#define DEBUG_ENABLED false
 #endif // DEBUG

 namespace Udjat {

	namespace Logger {

		class UDJAT_PRIVATE Stream : public std::basic_streambuf<char, std::char_traits<char> > {
			private:
				Level level;

			public:
				class Buffer : public std::string {
				private:
					Level level;
					Buffer(Level level);

				public:
					static Buffer & getInstance(Level l);
					~Buffer();

					Buffer(const Buffer &src) = delete;
					Buffer(const Buffer *src) = delete;

					bool push_back(int c);

					void sync();

				};

				Stream(Level l) : level(l) {
				}

				~Stream();

				/// @brief Writes characters to the associated file from the put area
				int sync() override;

				/// @brief Writes characters to the associated output sequence from the put area.
				int overflow(int c) override;

		};

		class UDJAT_PRIVATE Controller {
			private:
				std::recursive_mutex guard;
				Controller();

				/// @brief Enabled/disabled log types
				bool levels[Level::Count] = {
					true,	// Error conditions (std::cerr).
					true,	// Warning conditions (std::clog).
					true,	// Informational message (std::cout).
					false,	// Debug message.
					false,	// Trace message
					true,	// System Status
				};

				/// @brief Log writer callback.
				struct Writer {
					const char *name;
					std::function<void(Level level, const char *timestamp, const char *domain, const char *text)> call;
					Writer(const char *n,const std::function<void(Level level, const char *timestamp, const char *domain, const char *text)> &c) : name(n), call(c) { }
				};

				std::list<Writer> writers;

#ifndef _WIN32

				typedef enum {
					// log flags
					G_LOG_FLAG_RECURSION	= 1 << 0,
					G_LOG_FLAG_FATAL		= 1 << 1,

					// GLib log levels
					G_LOG_LEVEL_ERROR		= 1 << 2,       /* always fatal */
					G_LOG_LEVEL_CRITICAL	= 1 << 3,
					G_LOG_LEVEL_WARNING		= 1 << 4,
					G_LOG_LEVEL_MESSAGE		= 1 << 5,
					G_LOG_LEVEL_INFO		= 1 << 6,
					G_LOG_LEVEL_DEBUG		= 1 << 7,

					G_LOG_LEVEL_MASK		= ~(G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL)
				} GLogLevelFlags;

				/// @brief Handler for glib/gtk log messages.
				static void g_logger(const char *domain, GLogLevelFlags level, const char *message, void *userdata);

#endif // !_WIN32

			public:
				Controller(const Controller &src) = delete;
				Controller(const Controller *src) = delete;

				static Controller & getInstance();
				~Controller();
			
				void insert(const char *name,const std::function<void(Level level, const char *timestamp, const char *domain, const char *text)> &call);
				void remove(const char *name);				

				void write(Level level, const char *domain, const char *text);

				void setup(const Properties &props);

				/// @brief Enable/disable log messages.
				/// @param level The message type to enable/disable.
				/// @param enable The new state for the message type.
				inline void enable(Level level, bool enable = true) noexcept {
					levels[level] = enable;
				}
					
				inline bool enabled(Level level) const noexcept {
					return levels[level % Level::Count];
				}

				void console(bool enable = true) noexcept;
				void file(const char *filename = nullptr, time_t max_age = 86400) noexcept;

		};

		/*
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

*/

	}

 }
