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
 #include <udjat/tools/xml.h>
 #include <cstdarg>
 #include <udjat/tools/quark.h>
 #include <sstream>
 #include <iomanip>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	String::String(const XML::Node &node, const char *attrname, bool required)
		: String{XML::StringFactory(node,attrname,required ? nullptr : "")} {
		expand(node);
	}

	String::String(const XML::Node &node, const char *attrname, const char *def)
		: String{XML::StringFactory(node,attrname,def)} {
		expand(node);
	}

 }

