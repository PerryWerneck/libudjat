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


#include <cstring>
#include <udjat/tools/timestamp.h>

using namespace std;

std::string Udjat::TimeStamp::to_string(const char *format) const noexcept {

	if(!value)
		return "";

	char timestamp[80];
	memset(timestamp,0,sizeof(timestamp));

	struct tm tm;

	memcpy(&tm,localtime(&value),sizeof(tm));
	strftime(timestamp, 79, format, &tm);

	return std::string(timestamp);
}


