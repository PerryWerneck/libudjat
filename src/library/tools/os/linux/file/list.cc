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
 #include <sys/types.h>
 #include <udjat/tools/file/list.h>

 #include "private.h"

 using namespace std;

 namespace Udjat {


	File::List::List(const char *fpath, bool recursive) : List(fpath,"*",recursive) {
	}

	File::List::List(const char *path, const char *pattern, bool recursive) {

		File::Path{path}.for_each([this,pattern](const File::Path &filename){
			if(filename.match(pattern)) {
				emplace_back(filename);
			}
			return false;
		},recursive);

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
