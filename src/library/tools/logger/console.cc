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
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
 	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	static const char * decoration(Logger::Level level) noexcept {
		static const char *decorations[Logger::Level::Count] = {
			"\x1b[91m",	// Error
			"\x1b[93m",	// Warning
			"\x1b[92m",	// Info
			"\x1b[94m",	// Trace
			"\x1b[95m",	// Debug
			"\x1b[96m",	// Notice
		};
		return decorations[((size_t) level) % Udjat::Logger::Level::Count];
	}

#ifdef _WIN32

	static bool decorated(HANDLE hOut) noexcept {
		DWORD mode = 0;
		if(GetConsoleMode(hOut, &mode)) {
			return (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
		}
		return false;	
	}

	static bool write_text(HANDLE hOut,const char *text) {
		DWORD bytes = strlen(text);
		while(bytes) {
			DWORD sz = 0;
			if(!WriteFile(hOut,text,bytes,&sz,NULL)) {
				return false;
			}
			bytes -= sz;
			text += sz;
		}
		return true;
	}

#else

	static bool write_text(const char *text) {
		size_t bytes = strlen(text);
		while(bytes) {
			ssize_t sz = write(1,text,bytes);
			if(sz < 0)
				return false;
			bytes -= sz;
			text += sz;
		}
		return true;
	}

#endif // !_WIN32

	UDJAT_API bool Logger::decorated() noexcept {
#ifdef _WIN32
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if(hOut != INVALID_HANDLE_VALUE) {
			return Udjat::decorated(hOut);
		}
		return false;	
#else
		static bool flag = isatty(1) && (getenv("TERM") != NULL);
		return flag;
#endif // _WIN32
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

			auto dec = Udjat::decorated(hOut);

			if(dec) {
				write_text(hOut,decoration(level));
			}

			write_text(hOut,timestamp);
			write_text(hOut," ");

			char domain_buffer[11];
			memset(domain_buffer,' ',sizeof(domain_buffer));
			memcpy(domain_buffer,domain,std::min(sizeof(domain_buffer)-1,strlen(domain)));
			domain_buffer[sizeof(domain_buffer)-1] = 0;
			
			write_text(hOut,domain_buffer);
			write_text(hOut," ");
			write_text(hOut,text);

			if(dec) {
				write_text(hOut,"\x1b[0m");
			}

			write_text(hOut,"\r\n");
		});
#else
		// Insert linux console backend
		insert("console",BackEnd::Console,[](Level level, const char *timestamp, const char *domain, const char *text) {

			bool dec = decorated();

			if(dec) {
				write_text(decoration(level));
			}

			write_text(timestamp);
			write_text(" ");

			char domain_buffer[11];
			memset(domain_buffer,' ',sizeof(domain_buffer));
			memcpy(domain_buffer,domain,std::min(sizeof(domain_buffer)-1,strlen(domain)));
			domain_buffer[sizeof(domain_buffer)-1] = 0;
			
			write_text(domain_buffer);
			write_text(" ");
			write_text(text);

			if(dec) {
				write_text("\x1b[0m");
			}

			write_text("\r\n");
			fsync(1);

		});

#endif // _WIN32
	}

 }

