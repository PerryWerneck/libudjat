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

#include <windows.h>
#include <config.h>

#include <udjat/tools/logger.h>
#include <udjat/tools/timestamp.h>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

namespace Udjat {

	Logger::~Logger() {
	}

	Logger::Message & Logger::Message::append(const char *value) {

		size_t from = find("{");
		if(from == std::string::npos) {
			throw std::runtime_error("Invalid template string");
		}

		size_t to = find("}",from);
		if(to == std::string::npos) {
			throw std::runtime_error("Invalid template string");
		}

		replace(from,(to-from)+1,value);

		return *this;
	}

	Logger::Message & Logger::Message::append(const std::exception &e) {
		return append(e.what());
	}

	void Logger::set(const pugi::xml_node &node) {
		auto attribute = node.attribute("name");
		if(attribute)
			name = Quark(attribute.as_string(this->name)).c_str();
	}

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
				char timestamp[80];
				{
					time_t t = time(0);
					struct tm tm = *localtime(&t);

					memset(timestamp,0,sizeof(timestamp));

					size_t len = strftime(timestamp, 79, "%x %X", &tm);

					if(!len) {
						strncpy(timestamp,"--/--/-- --:--:--", sizeof(timestamp));
					}

				}

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
					write(1,timestamp,module,message);
				}

				// Write to log file.
				{

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

}

