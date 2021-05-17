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

namespace Udjat {

	const char * Abstract::State::levelnames[] = {
		"undefined",
		"unimportant",
		"ready",
		"warning",
		"error",
		"critical",

		nullptr
	};

	Abstract::State::Level Abstract::State::getLevelFromName(const char *name) {

		for(size_t ix=0; levelnames[ix]; ix++) {
			if(!strcasecmp(name,levelnames[ix]))
				return (Abstract::State::Level) ix;
		}

		throw runtime_error(string{"Unknown level '"} + name + "'");

	}

	void Abstract::State::getLevel(Json::Value &value) const {
		Json::Value level;
		level["value"] = (uint32_t) this->level;
		level["label"] = levelnames[this->level];
		value["level"] = level;
	}

	const char * Abstract::State::to_string(const Abstract::State::Level level) {

		if(level > (sizeof(levelnames) / sizeof(levelnames[0])))
			return "Invalid";
		return levelnames[level];
	}


}
