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

 #include <udjat/defs.h>
 #include <udjat/tools/expander.h>
 #include <cstring>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	UDJAT_API const char * expand(std::string &text, const Expander &exec) {

		auto from = text.find("${");
		while(from != string::npos) {

			auto to = text.find("}",from+3);
			if(to == string::npos) {
				throw runtime_error("Invalid ${} usage");
			}

			string value;
			if(exec(string(text.c_str()+from+2,(to-from)-2).c_str(),value)) {

				// Got value, apply it.
				text.replace(
					from,
					(to-from)+1,
					value.c_str()
				);

				from = text.find("${",from);

			} else {

				// No value, skip.
				from = text.find("${",to+1);

			}

		}

		return text.c_str();
	}

 }
