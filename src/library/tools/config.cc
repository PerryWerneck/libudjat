/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Brief Implements common config methods.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/configuration.h>
 #include <cstdarg>
 #include <string>
 #include <cstring>

 using namespace std;

 namespace Udjat {

	namespace Config {

		int Value<string>::select(const char *value, va_list args) const noexcept {

			if(empty()) {
				return -(errno = ENODATA);
			}

			int index = 0;
			while(value) {

				if(!strcasecmp(c_str(),value)) {
					va_end(args);
					return index;
				}

				index++;
				value = va_arg(args, const char *);
			}
			return -(errno = ENOENT);

		}

		int Value<string>::select(const char *value, ...) const noexcept {

			va_list args;
			va_start(args, value);
			int rc = select(value,args);
			va_end(args);

			return rc;

		}

	}

 }
