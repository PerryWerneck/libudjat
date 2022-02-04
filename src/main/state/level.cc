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

 #include "private.h"


 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

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

	Level LevelFactory(const pugi::xml_node &node) {
		return LevelFactory(node.attribute("level").as_string("unimportant"));
	}

	Udjat::Level LevelFactory(const char *name) {

		for(size_t ix=0; ix < LEVEL_COUNT; ix++) {
			if(!strcasecmp(name,levelnames[ix]))
				return (Udjat::Level) ix;
		}

		throw runtime_error(string{"Unknown level '"} + name + "'");

	}

}

namespace std {

	const char * to_string(const Udjat::Level level) {
		if(level > LEVEL_COUNT)
			return "undefined";
		return levelnames[level];
	}

}
