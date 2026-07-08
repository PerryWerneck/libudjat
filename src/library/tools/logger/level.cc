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
 #include <udjat/tools/intl.h>
 
 #ifdef HAVE_UNISTD_H
 	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	const Logger::Levels Logger::levels[LOGGER_MAX_VERBOSITY] = {
		{ Logger::Level::None,		N_("none")		},
		{ Logger::Level::Notice,	N_("notice")	},
		{ Logger::Level::Error,		N_("error")		},
		{ Logger::Level::Warning,	N_("warning")	},
		{ Logger::Level::Info,		N_("info")		},
		{ Logger::Level::Trace,		N_("trace")		},
		{ Logger::Level::Debug,		N_("debug")		},
	};

	bool Logger::enabled(Logger::Level level) noexcept {
		return Logger::Controller::getInstance().enabled(level);
	}

	void Logger::enable(Logger::Level level, bool enabled) noexcept {
		Logger::Controller::getInstance().enable(level);
	}

	int Logger::verbosity() noexcept {
		return (int) Logger::Controller::getInstance().verbosity();
	}

	int Logger::Controller::verbosity() const noexcept {
		int rc = Logger::Level::None;
		for(int ix=0; ix < LOGGER_MAX_VERBOSITY; ix++) {
			if(enabled_levels&levels[ix].level) {
				rc = ix;
			}
		}
		return rc;
	}

	void Logger::verbosity(int level) noexcept {
		Logger::Controller::getInstance().verbosity((size_t) level);
	}

	void Logger::Controller::verbosity(int level) noexcept {
		lock_guard<recursive_mutex> lock(guard);

		if(level > LOGGER_MAX_VERBOSITY) {
			level = LOGGER_MAX_VERBOSITY;
		}

		enabled_levels = Logger::Level::None;
		for(int ix=0; ix < level; ix++) {
			enabled_levels = (Logger::Level) (enabled_levels|levels[ix].level);
		}

	}

	void Logger::verbosity(const char *level) noexcept {
		Logger::Controller::getInstance().verbosity(level);
	}

	void Logger::Controller::verbosity(const char *level) {

		lock_guard<recursive_mutex> lock(guard);

		if(*level >= '0' && *level <= '9') {
			Logger::Controller::getInstance().verbosity(atoi(level));
			return;
		}
		
		enabled_levels = Logger::Level::None;
		for(auto &lvl : String{level}.split(",")) {
			for(size_t ix=0; ix < LOGGER_MAX_VERBOSITY; ix++) {
				if(!strcasecmp(levels[ix].name,lvl.c_str())) {
					enabled_levels = (Level) (levels[ix].level|enabled_levels);
				}
			}
		}

	}

	Logger::Level Logger::LevelFactory(const Properties &props, const char *attr, const char *def) {
		return LevelFactory(props.get(attr,def).c_str());
	}

	Logger::Level Logger::LevelFactory(const char *name) {
		for(size_t ix=0; ix < LOGGER_MAX_VERBOSITY; ix++) {
			if(!strcasecmp(levels[ix].name,name)) {
				return levels[ix].level;
			}
		}
		throw logic_error(String{"Unexpected log level '",name,"'"});
	}

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Logger::Level level) {

		const char *name = Udjat::Logger::levels[0].name;

		if(level) {
			for(size_t ix=0; ix < LOGGER_MAX_VERBOSITY; ix++) {
				if(Udjat::Logger::levels[ix].level & level) {
					name = Udjat::Logger::levels[ix].name;
				}
			}
		}
 
#ifdef GETTEXT_PACKAGE
		return dgettext(GETTEXT_PACKAGE,name);
#else
		return name;
#endif // GETTEXT_PACKAGE
	}

 }
