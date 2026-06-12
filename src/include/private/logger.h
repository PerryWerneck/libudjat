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
 #include <udjat/tools/string.h>
 #include <mutex>
 #include <functional>
 #include <list>
 #include <string>
 #include <thread>

 #ifdef DEBUG
	#define DEBUG_ENABLED true
 #else
	#define DEBUG_ENABLED false
 #endif // DEBUG

 namespace Udjat {

	namespace Logger {

		UDJAT_PRIVATE bool write(int fd, const char *text) noexcept;

		/// @brief Log writer callback.
		class UDJAT_PRIVATE BackEnd {
			public:
				const char *name;

				enum Type : uint8_t {
					Custom,
					Console,
					File,
					SysLog,

					Count
				};
				
				Type type = Custom;

				std::function<void(Level level, const char *timestamp, const char *domain, const char *text)> call;
				BackEnd(const char *n, const Type t,const std::function<void(Level level, const char *timestamp, const char *domain, const char *text)> &c) : name{n}, type{t}, call{c} { }
		};

		class UDJAT_PRIVATE Stream : public std::basic_streambuf<char, std::char_traits<char> > {
			public:
				class Buffer : public String {
				public:
					Level level;
					pthread_t thread;

					Buffer(Level level);
					~Buffer();

					static Buffer & getInstance(Level l);

					bool push_back(int c);

					/// @brief Send contents to logger, clear string.
					void send();

				};

			private:
				Level level;
				static std::mutex guard;	
				static std::list<Buffer> buffers;

				Buffer & getBuffer(Level level);

				/// @brief Send contents to logger, remove buffer.
				void send();

			public:

				Stream(Level l) : level{l} {
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

				std::list<BackEnd> backends;

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
			
				void insert(const char *name,const BackEnd::Type type,const std::function<void(Level level, const char *timestamp, const char *domain, const char *text)> &call);
				void remove(const char *name);				

				void write(Level level, const char *domain, const char *text);

				void setup(const Properties &props);

				bool enabled(BackEnd::Type type) const noexcept;
					
				/// @brief Enable/disable log messages.
				/// @param level The message type to enable/disable.
				/// @param enable The new state for the message type.
				inline void enable(Level level, bool enable = true) noexcept {
					levels[level] = enable;
				}
					
				inline bool enabled(Level level) const noexcept {
					return levels[level % Level::Count];
				}

				void file(const char *filename = nullptr, time_t max_age = 86400) noexcept;
				void console(bool enable);

		};

	}

 }
