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
 #include <unistd.h>
 #include <cstring>
 #include <udjat/defs.h>
 #include <udjat/tools/network.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	static std::string hostname() noexcept {

		char buffer[255];
		DWORD bufCharCount = sizeof(buffer) -1;

		if(GetComputerName(buffer,&bufCharCount)) {
			buffer[bufCharCount] = 0;
			char *ptr = strchr(buffer,'.');
			if(ptr) {
				*ptr = 0;
			}
			return buffer;
		}

		if(gethostname(buffer,sizeof(buffer)) == 0) {
			char *ptr = strchr(buffer,'.');
			if(ptr) {
				*ptr = 0;
			}
			return buffer;
		}

		cerr << "Unable to get system hostname" << endl;

		return "";

	}

	Hostname::Hostname() : std::string{hostname()} {
	}

 }
