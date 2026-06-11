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
 #include <private/logger.h>
 #include <udjat/tools/properties.h>
 #include <stdexcept>
 
 #ifdef HAVE_UNISTD_H
 	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 static const char * levelnames[Udjat::Logger::Level::Count] = {
	"error",
	"warning",
	"info",
	"trace",
	"debug",
	"notice",
 };

 namespace Udjat {

	bool Logger::enabled(Logger::Level level) noexcept {
		return Logger::Controller::getInstance().enabled(level);
	}

	void Logger::enable(Logger::Level level, bool enabled) noexcept {
		Logger::Controller::getInstance().enable(level);
	}

	void Logger::verbosity(unsigned short level) {
		for(unsigned short ix = 0; ix < Logger::Level::Count; ix++) {
			enable((Level) ix,level > ix);
		}
	}

	void Logger::verbosity(const char *level) {

		if(*level >= '0' && *level <= '9') {
			verbosity((unsigned short) atoi(level));
			return;
		}

		for(auto &lvl : String{level}.split(",")) {
			lvl.strip();
			if(!lvl.empty()) {
				continue;
			}
			for(uint8_t ix = 0; ix < Logger::Level::Count; ix++) {
				if(!strcasecmp(levelnames[ix],lvl.c_str())) {
					enable((Level) ix,true);
				}
			}
		}

	}

	Logger::Level Logger::LevelFactory(const Properties &props, const char *attr, const char *def) {
		return LevelFactory(props.get(attr,def).c_str());
	}

	Logger::Level Logger::LevelFactory(const char *name) {
		for(uint8_t ix = 0; ix < Logger::Level::Count; ix++) {
			if(!strcasecmp(levelnames[ix],name)) {
				return (Logger::Level) ix;
			}
		}
		throw logic_error(String{"Unexpected log level '",name,"'"});
	}

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Logger::Level level) {
		return levelnames[((size_t) level) % Udjat::Logger::Level::Count];
	}

 }
