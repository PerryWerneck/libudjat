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

#ifndef _WIN32

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

	UDJAT_API void Logger::console(bool enable) {
		Controller::getInstance().console(enable);
	}

	void Logger::Controller::console(bool enable) noexcept {

		if(!enable) {
			remove("console");
			return;
		}

#ifdef _WIN32		
		// Insert win32 console writer
		#error TODO

#else
		// Insert linux console writer
		insert("console",[](Level level, const char *timestamp, const char *domain, const char *text) {

			bool dec = decorated();

			if(dec) {
				write_text(decoration(level));
			}

			write_text(timestamp);
			write_text(" ");
			write_text(domain);
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

