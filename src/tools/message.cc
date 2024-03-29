/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/message.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	void Message::append(const char *str) {

		string key{"{"};
		key += std::to_string(++index);
		key += "}";

		debug("Key=",key," value=",str);

		size_t from = find(key);
		if(from != std::string::npos) {
			replace(from,key.size(),str);
			return;
		}

		from = find("{}");

		if(from == std::string::npos) {
			cerr << "logger\tInvalid template appending '" << str << "' on '" << c_str() << "'" << endl;
			throw std::runtime_error(_("The message template is invalid"));
		}

		replace(from,2,str);

		return;
	}

 }

