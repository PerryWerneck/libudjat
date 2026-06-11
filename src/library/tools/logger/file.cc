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
 #include <memory>
 #include <udjat/tools/string.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <cstdio>
 #include <sys/stat.h>
 #include <fcntl.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	UDJAT_API void Logger::file(const char *filename, time_t max_age) {
		Controller::getInstance().file(filename,max_age);
	}

	bool Logger::write(int fd, const char *text) noexcept {
		size_t bytes = strlen(text);
		while(bytes) {
			ssize_t sz = ::write(fd,text,bytes);
			if(sz < 0)
				return false;
			bytes -= sz;
			text += sz;
		}
		return true;
	}

	void Logger::Controller::file(const char *filename, time_t max_age) noexcept {

		if(!(filename && *filename)) {
			remove("file");
			return;
		}

		// Filename template.
		String tmplt{filename};

		tmplt.expand([](const char *key, std::string &value){
			if(!strcasecmp(key,"logdir")) {
				value = Application::LogDir::getInstance().c_str();
			}
			return false;
		},false,false);

		// Insert file writer.
		insert("file",BackEnd::File,[tmplt,max_age](Level, const char *timestamp, const char *domain, const char *text) {

			String filename{TimeStamp{}.to_string(tmplt)};
			filename.expand();
			
			// Remove file if it's too old.
			if(max_age){
				struct stat st;
				if(!stat(filename.c_str(),&st) && (time(nullptr) - st.st_mtime) > max_age) {
					// File is too old, remove it.
					::remove(filename.c_str());
				}
			}
			
			int fd = ::open(filename.c_str(),O_WRONLY|O_APPEND|O_CREAT,0664);
			if(fd < 0) {
				throw system_error(errno,system_category(),filename);
			}

			Logger::write(fd,timestamp);
			Logger::write(fd,"\t");
			Logger::write(fd,domain);
			Logger::write(fd,"\t");
			Logger::write(fd,text);
			Logger::write(fd,"\n");

			::close(fd);

		});

	}

 }

