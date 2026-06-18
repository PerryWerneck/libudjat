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

 #include <config.h>
 #include <udjat/tools/logger.h>
 #include <private/logger.h>
 #include <udjat/tools/properties.h>
 #include <udjat/ui/console.h>
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
 	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	using Console = UI::Console;

	static const char * decoration(Logger::Level level) noexcept {

		static const struct {
			Logger::Level level;
			const char *decoration;
		} decorations[] = {
			{ Logger::Level::Error, 	"\x1b[91m" },
			{ Logger::Level::Notice, 	"\x1b[96m" },
			{ Logger::Level::Warning,	"\x1b[93m" },
			{ Logger::Level::Info, 		"\x1b[92m" },
			{ Logger::Level::Trace, 	"\x1b[94m" },
			{ Logger::Level::Debug, 	"\x1b[95m" },
		};

		for(const auto &decoration : decorations) {
			if(decoration.level & level) {
				return decoration.decoration;
			}
		}

		return "";
	}


	UDJAT_API void Logger::console(bool enable) {
		Controller::getInstance().console(enable);
	}

	UDJAT_API bool Logger::console() {
		return Controller::getInstance().enabled(BackEnd::Console);
	}

	void Logger::Controller::console(bool enable) {

		if(!enable) {
			remove("console");
			return;
		}

#ifdef _WIN32		
		// Is the win32 console available?
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if(hOut == INVALID_HANDLE_VALUE) {
			return;
		}

		// Yes, insert handle.
		insert("console",BackEnd::Console,[hOut](Level level, const char *timestamp, const char *domain, const char *text) {

			auto dec = Console::decorated(hOut);

			if(dec) {
				Console::write(hOut,decoration(level));
			}

			Console::write(hOut,timestamp);
			Console::write(hOut," ");

			char domain_buffer[11];
			memset(domain_buffer,' ',sizeof(domain_buffer));
			memcpy(domain_buffer,domain,std::min(sizeof(domain_buffer)-1,strlen(domain)));
			domain_buffer[sizeof(domain_buffer)-1] = 0;
			
			Console::write(domain_buffer);
			Console::write(hOut," ");
			Console::write(hOut,text);

			if(dec) {
				Console::write(hOut,"\x1b[0m");
			}

			Console::write(hOut,"\r\n");
		});
#else
		// Insert linux console backend
		insert("console",BackEnd::Console,[](Level level, const char *timestamp, const char *domain, const char *text) {

			bool dec = Console::decorated();

			if(dec) {
				Console::write(decoration(level));
			}

			Console::write(timestamp);
			Console::write(" ");

			char domain_buffer[11];
			memset(domain_buffer,' ',sizeof(domain_buffer));
			memcpy(domain_buffer,domain,std::min(sizeof(domain_buffer)-1,strlen(domain)));
			domain_buffer[sizeof(domain_buffer)-1] = 0;
			
			Console::write(domain_buffer);
			Console::write(" ");
			Console::write(text);

			if(dec) {
				Console::write("\x1b[0m");
			}

			Console::write("\r\n");
			fsync(1);

		});

#endif // _WIN32
	}

 }

