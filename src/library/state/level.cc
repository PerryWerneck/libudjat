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
 #include <udjat/tools/xml.h>
 #include <udjat/agent/level.h>
 #include <cstring>

 using namespace std;

 static const char * levelnames[] = {
	"undefined",
	"unimportant",
	"ready",
	"warning",
	"error",
	"critical"
 };

 #define LEVEL_COUNT (sizeof(levelnames)/sizeof(levelnames[0]))

 namespace Udjat {

	Udjat::Level LevelFactory(const Properties &props) {
		return LevelFactory(props.get("level","unimportant").c_str());
	}

	Udjat::Level LevelFactory(const char *name) {

		for(size_t ix=0; ix < LEVEL_COUNT; ix++) {
			if(!strcasecmp(name,levelnames[ix]))
				return (Udjat::Level) ix;
		}

		throw runtime_error(string{"Unknown level '"} + name + "'");

	}

	Logger::Level LogLevelFactory(const Level level) {

		static const struct {
			Level from;
			Logger::Level to;
		} values[] = {

			{ Level::undefined,		Logger::Trace 	},
			{ Level::unimportant,	Logger::Trace	},
			{ Level::ready,			Logger::Info	},
			{ Level::warning,		Logger::Warning	},
			{ Level::error,			Logger::Error	},
			{ Level::critical,		Logger::Error	},

		};

		for(const auto &value : values) {
			if(value.from == level) {
				return value.to;
			}
		}

		return Logger::Error;
	}


 }

 namespace std {

	const char * to_string(const Udjat::Level level) {
		if(level > LEVEL_COUNT)
			return "undefined";
		return levelnames[level];
	}

 }
