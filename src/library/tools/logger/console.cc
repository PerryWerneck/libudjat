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
 #include <udjat/tools/configuration.h>
 #include <udjat/ui/console.h>
 #include <stdexcept>

 #ifdef HAVE_UNISTD_H
 	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	UDJAT_API bool Logger::console(bool enable) {
		auto &controller = Controller::getInstance();
		bool rc = controller.enabled(BackEnd::Console);
		Controller::getInstance().console(enable);
		return rc;
	}

	UDJAT_API bool Logger::console() {
		return Controller::getInstance().enabled(BackEnd::Console);
	}

	void Logger::Controller::console(bool enable) {

		remove("console");
		if(!enable) {
			return;
		}

#ifdef _WIN32		
		// Is the win32 console available? If not just return with no action.
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if(hOut == INVALID_HANDLE_VALUE) {

			// FIXME: Test if AllocConsole() can be used to spawn a console window.

			return;
		}

		static bool initialized = false;
		if(!initialized) {

			// https://github.com/alf-p-steinbach/Windows-GUI-stuff-in-C-tutorial-/blob/master/docs/part-04.md
			SetConsoleOutputCP(CP_UTF8);
			SetConsoleCP(CP_UTF8);

			if(Config::Value<bool>("application","virtual-terminal-processing",true)) {
				// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
				DWORD dwMode = 0;
				if(GetConsoleMode(hOut, &dwMode)) {
					dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
					SetConsoleMode(hOut,dwMode);
				}
			}

		}
#endif // _WIN32

		// Insert console backend
		bool decorated = Console::decorated();
		
		insert("console",BackEnd::Console,[decorated](Level level, const char *timestamp, const char *domain, const char *text) {

			if(decorated) {
				// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
				Console::write("\r\x1b[2K");
				Console::write(Console::color(level));
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

			if(decorated) {
				Console::write("\x1b[0m");
			}

			Console::write("\r\n");

#ifndef _WIN32
			fsync(1);
#endif // !_WIN32

		});

	}

 }

