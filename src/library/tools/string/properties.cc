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
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/properties.h>
 #include <cstdarg>
 #include <udjat/tools/quark.h>
 #include <sstream>
 #include <iomanip>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	String::String(const Properties &props, const char *attrname, bool required) {
		if(required && !props.contains(attrname)) {
			throw runtime_error(String{"Required attribute '",attrname,"' is missing"});
		}
		assign(props[attrname]);
		expand(props);
	}

	String::String(const Properties &props, const char *attrname, const char *def)
		: String{props.get(attrname,def)} {
		expand(props);
	}

 }

