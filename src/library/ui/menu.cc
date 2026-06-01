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

 /**
  * @brief Implements menu dialog.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/menu.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>
 #include <vector>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif

 using namespace std;

 namespace Udjat {

	Dialog::Menu::Menu(const char *t) : title{t} {
	}

	Dialog::Menu::~Menu() {
	}

	size_t Dialog::Menu::select(const Option *options, size_t count) {
		vector<Option> opt;
		for(size_t ix = 0; ix < count;ix++) {
			opt.push_back(Option{options[ix].text});
		}
		return select(opt);
	}

	size_t Dialog::Menu::select(const Option *options) {
		vector<Option> opt;
		for(size_t ix = 0; options[ix].text;ix++) {
			opt.push_back(Option{options[ix].text});
		}
		return select(opt);
	}


}



