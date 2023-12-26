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

 #pragma once

 #include <udjat/defs.h>
 #include <string>
 #include <cstdlib>

 namespace Udjat {

	inline int to_value(const char *str, const int) {
		return std::stoi(str);
	}

	inline unsigned int to_value(const char *str, const unsigned int) {
		return (unsigned int) std::stoul(str);
	}

	inline short to_value(const char *str, const short) {
		return (short) std::stoi(str);
	}

	inline unsigned short to_value(const char *str, const unsigned short) {
		return (unsigned short) std::stoi(str);
	}

	inline long to_value(const char *str, const long) {
		return std::stol(str);
	}

	inline unsigned long to_value(const char *str, const unsigned long) {
		return std::stoul(str);
	}

	inline long long to_value(const char *str, const long long) {
		return std::stoll(str);
	}

	inline unsigned long long to_value(const char *str, const unsigned long long) {
		return std::stoull(str);
	}

	inline float to_value(const char *str, const float) {
		return std::stof(str);
	}

	inline double to_value(const char *str, const double) {
		return std::stod(str);
	}

	inline int to_value(const pugi::xml_node &node, const int value, const char *attrname = "value") {
		return node.attribute(attrname).as_int(value);
	}

	inline unsigned int to_value(const pugi::xml_node &node, const unsigned int value, const char *attrname = "value") {
		return node.attribute(attrname).as_uint(value);
	}

	inline unsigned short to_value(const pugi::xml_node &node, const unsigned short value, const char *attrname = "value") {
		return (unsigned short) node.attribute(attrname).as_int(value);
	}

	inline float to_value(const pugi::xml_node &node, const float value, const char *attrname = "value") {
		return node.attribute(attrname).as_float(value);
	}

	inline double to_value(const pugi::xml_node &node, const double value, const char *attrname = "value") {
		return node.attribute(attrname).as_double(value);
	}

	inline unsigned long to_value(const pugi::xml_node &node, const unsigned long value, const char *attrname = "value") {
		return (unsigned long) node.attribute(attrname).as_uint(value);
	}

	inline long to_value(const pugi::xml_node &node, const long value, const char *attrname = "value") {
		return (long) node.attribute(attrname).as_int(value);
	}

 }

