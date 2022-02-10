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
#include <udjat/win32/utils.h>
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

		// If enable write console output.
		if(console) {
			write(1,timestamp.c_str());
			write(1," ");
			write(1,module);
			write(1," ");
			write(1,message);
			write(1,"\n");
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


	/*
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Logger::redirect(const char *filename, bool console) {

		/// @brief Mutex para serialização.
		static std::mutex guard;

		class Writer : public std::basic_streambuf<char, std::char_traits<char> > {
		private:

			/// @brief Send output to console?
			bool console;
			/// @brief Buffer contendo a linha de log.
			std::string buffer;

			void write(int fd, const char *str) {

				size_t bytes = strlen(str);

				while(bytes > 0) {

					ssize_t sz = ::write(fd,str,bytes);
					if(sz < 0)
						return;
					bytes -= sz;
					str += sz;

				}

			}

			void write(int fd, const char *timestamp, const char *module, const char *message) {

				write(fd,timestamp);
				write(fd," ");
				write(fd,module);
				write(fd,"\t");
				write(fd,message);
				write(fd,"\n");

			}

		protected:

			/// @brief Writes characters to the associated file from the put area
			int sync() override {
				return 0;
			}

			void write() {

				lock_guard<std::mutex> lock(guard);

				// Remove spaces
				size_t len = buffer.size();
				while(len--) {

					if(isspace(buffer[len])) {
						buffer[len] = 0;
					} else {
						break;
					}
				}

				buffer.resize(strlen(buffer.c_str()));

				if(buffer.empty()) {
					return;
				}

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
						if(length >= sizeof(module)) {
							length = sizeof(module);
						}
						strncpy(module,message,length);
						message = (ptr+1);

					}

					module[sizeof(module)-1] = 0;

				}

				// If enable write console output.
				if(console) {
					write(1,timestamp.c_str(),module,message);
				}

				// Write to log file.
				{
					static Application::LogDir path;
					static string format;

					if(format.empty()) {
						format = Application::Name();
						format += "-%d.log";
					}

					// Get logfile path.
					string filename = path;
					filename.append(TimeStamp().to_string(format.c_str()));

					struct stat st;
					if(!stat(filename.c_str(),&st) && (time(nullptr) - st.st_mtime) > 86400) {
						// Tem mais de um dia, remove
						remove(filename.c_str());
					}

					std::ofstream ofs;
					ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);

					ofs.open(filename, ofstream::out | ofstream::app);
					ofs << timestamp << " " << module << " " << message << endl;
					ofs.close();

				}

				buffer.erase();

			}


			/// @brief Writes characters to the associated output sequence from the put area.
			int overflow(int c) override {

				if(c == EOF || c == '\n' || c == '\r') {
					write();
				} else {
					lock_guard<std::mutex> lock(guard);
					buffer += static_cast<char>(c);
				}

				return c;
			}

		public:
			Writer(bool c) : console(c) {
			}

			~Writer() {
			}

		};

		std::clog.rdbuf(new Writer(console));
		std::cout.rdbuf(new Writer(console));
		std::cerr.rdbuf(new Writer(console));

	}
	#pragma GCC diagnostic pop
	*/
}

