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
#include <udjat-internals.h>
#include <cstring>
#include <udjat/tools/timestamp.h>
#include <udjat/tools/logger.h>
#include <iostream>

using namespace std;

namespace Udjat {

	std::string TimeStamp::to_string(const char *format) const noexcept {

		if(!value)
			return "";

		char timestamp[80];
		memset(timestamp,0,80);

		struct tm tm;

#ifdef HAVE_LOCALTIME_R
		localtime_r(&value,&tm);
#else
		tm = *localtime(&value);
#endif // HAVE_LOCALTIME_R

		size_t len = strftime(timestamp, 79, format, &tm);
		if(len == 0) {
			return "";
		}

		return std::string(timestamp,len);
	}

	TimeStamp & TimeStamp::set(const char *time, const char *format) {

		struct tm t;
		memset(&t,0,sizeof(t));

		if(format && *format) {

			// Have format, use-it
			if(!strptime(time, format, &t)) {
				throw runtime_error(string{"Can't parse '"} + time + "' in the requested format");
			}

		} else {

			// Dont have format, try known ones.
			static const char *formats[] = {
				"%Y-%m-%d %T",
				"%y-%m-%d %T",
				"%x %X",
			};

			bool found = false;
			for(size_t ix = 0; ix < (sizeof(formats)/sizeof(formats[0])); ix++) {
				if(strptime(time,formats[ix],&t)) {
					found = true;
					break;
				}
			}

			if(!found) {
				throw runtime_error(string{"Can't parse '"} + time + "' in any known format");
			}

		}

#ifdef DEBUG
		cout << "day=" << t.tm_mday << " month=" << t.tm_mon << " Year=" << t.tm_year << endl;
#endif // DEBUG

		this->value = mktime(&t);

		return *this;
	}

}
