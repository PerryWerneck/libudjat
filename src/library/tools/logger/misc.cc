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

 static const char * levelnames[Udjat::Logger::Level::Count] = {
	"error",
	"warning",
	"info",
	"trace",
	"debug",
	"notice",
 };

 namespace Udjat {

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

	Logger::Level Logger::LevelFactory(const char *name) noexcept {
		for(uint8_t ix = 0; ix < Logger::Level::Count; ix++) {
			if(!strcasecmp(levelnames[ix],name)) {
				return (Logger::Level) ix;
			}
		}
		throw logic_error(String{"Unexpected log level '",name,"'"});
	}

	bool Logger::enabled(Logger::Level level) noexcept {
		return Logger::Controller::getInstance().enabled(level);
	}

	void Logger::enable(Logger::Level level, bool enabled = true) noexcept {
		Logger::Controller::getInstance().enable(level);
	}


	bool Logger::decorated() noexcept {
#ifdef _WIN32
		return false;
#else
		static bool flag = isatty(1) && (getenv("TERM") != NULL);
		return flag;
#endif // _WIN32
	}

	void Logger::setup(const Properties &properties, const char *prefix) noexcept {

		auto &controller = Controller::getInstance();

		for(const auto &name : levelnames) {
			String attribute{prefix,name};
			if(properties.contains(attribute.c_str())) {
				controller.enable(LevelFactory(name),properties.get(attribute.c_str(),true));
			}
		}

		{
			String console{prefix,"console"};
			if(properties.contains(console.c_str())) {
				controller.console(properties.get(console.c_str(),true));
			}
		}

		{
			String file{prefix,"file"};
			if(properties.contains(file.c_str())) {
				controller.file(
					properties.get(file.c_str(),"").c_str(),
					properties.get(String{prefix,"max-age"}.c_str(),86400)
				);
			}
		}

		{
			String vb{prefix,"verbosity"};
			if(properties.contains(vb.c_str())) {
				verbosity(properties.get(vb.c_str(),"error").c_str());
			}
		}

	}

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Logger::Level level) {
		return levelnames[((size_t) level) % Udjat::Logger::Level::Count];
	}

 }
