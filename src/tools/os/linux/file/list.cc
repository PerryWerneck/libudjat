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
 #include <wordexp.h>

 using namespace std;

 namespace Udjat {

	File::List::List(const char *pattern) {

		wordexp_t p;
		memset(&p,0,sizeof(p));

		wordexp(pattern, &p, 0);

		list<string> lst;

		for(size_t wordc = 0; wordc < p.we_wordc; wordc++) {
			emplace_back(((char **) p.we_wordv)[wordc]);
		}

		wordfree(&p);

	}

	File::List::~List() {
	}

	bool File::List::for_each(std::function<bool (const char *filename)> call) {
		for(auto ix = begin(); ix != end(); ix++)  {
			if(!call( (*ix).c_str())) {
				return false;
			}
		}
		return true;
	}

 }
