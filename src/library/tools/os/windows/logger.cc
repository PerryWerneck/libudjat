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
#include <private/logger.h>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/application.h>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/configuration.h>
#include <udjat/win32/charset.h>
#include <udjat/win32/registry.h>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>

using namespace std;

namespace Udjat {

	void Logger::write(Level level, const char *domain, const char *text) noexcept {
		write(level,domain,text,false);
	}

	/// @brief Default console writer.
	void Logger::console_writer(Logger::Level level, const char *domain, const char *text) noexcept {

		// Log to console
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dunno;

		if(hOut != INVALID_HANDLE_VALUE) {

			const char *prefix = "";
			const char *suffix = "";

			DWORD mode = 0;
			if(GetConsoleMode(hOut, &mode)) {

				if(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {

					// Allowed, setup console colors.
					// https://learn.microsoft.com/pt-br/windows/console/console-virtual-terminal-sequences
					static const char *decorations[] = {
						"\x1b[91m",	// Error
						"\x1b[93m",	// Warning
						"\x1b[92m",	// Info
						"\x1b[94m",	// Trace
						"\x1b[95m",	// Debug

						"\x1b[96m",	// SysInfo (Allways Debug+1)
					};

					prefix = decorations[((size_t) level) % (sizeof(decorations)/sizeof(decorations[0]))];
					suffix = "\x1b[0m";

				}

			}

			if(*prefix) {
				WriteFile(hOut,prefix,strlen(prefix),&dunno,NULL);
			}

			{
				std::string timestamp{TimeStamp{}.to_string("%x %X")};
				WriteFile(hOut,timestamp.c_str(),timestamp.size(),&dunno,NULL);
			}

			WriteFile(hOut," ",1,&dunno,NULL);
			WriteFile(hOut,domain,sizeof(domain)-1,&dunno,NULL);
			WriteFile(hOut," ",1,&dunno,NULL);
			WriteFile(hOut,text,strlen(text),&dunno,NULL);

			if(*suffix) {
				WriteFile(hOut,suffix,strlen(suffix),&dunno,NULL);
			}

			WriteFile(hOut,"\r\n",2,&dunno,NULL);

		}

	}

	/// @brief Default file writer.
	void Logger::file_writer(Level, const char *domain, const char *text) noexcept {

		static string format;
		static unsigned int keep = 0;

		try {

			string filename{Application::LogDir::getInstance().c_str()};

			if(format.empty()) {

				try {

					keep = Config::Value<unsigned int>("logfile","max-age",86400).get();
					format = Config::Value<std::string>("logfile","name-format", (Application::Name() + "-%d.log").c_str()).c_str();

				} catch(...) {

					// On error assume defaults.
					keep = 86400;
					format = (Application::Name() + "-%d.log");

				}

			}

			// Current timestamp.
			TimeStamp timestamp;

			// Get logfile path.

			filename.append(timestamp.to_string(format.c_str()));

			struct stat st;
			if(!stat(filename.c_str(),&st) && (time(nullptr) - st.st_mtime) > keep) {
				// More than one day, remove it
				remove(filename.c_str());
			}

			// Open file
			std::ofstream ofs;
			ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);

			ofs.open(filename, ofstream::out | ofstream::app);
			ofs << timestamp << " " << domain << " " << text << endl;
			ofs.close();

		} catch(const std::exception &e) {

			console_writer(Logger::Error,"logger",e.what());
			std::abort();

		} catch(...) {

			console_writer(Logger::Error,"logger","Unexpected error");
			std::abort();

		}

	}

}

