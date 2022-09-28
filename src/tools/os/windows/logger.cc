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

#include <config.h>

#include <udjat/defs.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/application.h>
#include <udjat/win32/string.h>
#include <udjat/win32/registry.h>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

using namespace std;

namespace Udjat {

	void Logger::Writer::write(Buffer &buffer) {

		const char *message = buffer.c_str();

		// Set timestamp
		string timestamp = TimeStamp().to_string("%x %X");

		// Set module name.
		char module[12];
		{
			memset(module,' ',sizeof(module));

			const char *ptr = strchr(message,'\t');
			if(ptr) {

				// Has module name.
				size_t length = (ptr-message);
				if(length >= sizeof(module)-1) {
					length = sizeof(module)-1;
				}
				memcpy(module,message,length);
				message = (ptr+1);

			}

			module[sizeof(module)-1] = 0;

		}

		// If enabled write console output.
		if(console) {

			const char *prefix = "";
			const char *suffix = "";

			{
				DWORD mode = 0;
				HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

				if(hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode)) {

					if(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {

						// Allowed, setup console colors.
						static const char *decorations[] = {
							"\x1b[32m",	// Info
							"\x1b[33m",	// Warning
							"\x1b[91m"	// Error
						};

						if(this->id < (sizeof(decorations)/sizeof(decorations[0]))) {
							prefix = decorations[id];
						}

						suffix = "\x1b[0m";
					}

				}

			}

			fprintf(
					stdout,
					"%s%s %s %s%s\n",
					prefix,
					timestamp.c_str(),
					module,
					message,
					suffix
			);
			fflush(stdout);

		}

		//
		// Get filename.
		//
		string filename, format;
		DWORD keep = 86400;

		try {

			Win32::Registry registry("log");

			keep = registry.get("keep",keep);
			filename.assign(registry.get("path",""));
			format.assign(registry.get("format",""));

		} catch(...) {
			// On error assume defaults.
			filename.clear();
			format.clear();
		}

		if(filename.empty()) {
			filename.assign(Application::Path() + "\\logs\\");
		}

		if(format.empty()) {
			format.assign(Application::Name() + "-%d.log");
		}

		mkdir(filename.c_str());

		// Get logfile path.
		filename.append(TimeStamp().to_string(format.c_str()));

		struct stat st;
		if(!stat(filename.c_str(),&st) && (time(nullptr) - st.st_mtime) > keep) {
			// More than one day, remove it
			remove(filename.c_str());
		}

		// Open file
		std::ofstream ofs;
		ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);

		ofs.open(filename, ofstream::out | ofstream::app);
		ofs << timestamp << " " << module << " " << message << endl;
		ofs.close();

	}

}

