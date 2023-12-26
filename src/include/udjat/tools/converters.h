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

	inline int to_value(const char *str, int &value) {
		return (value = std::stoi(str));
	}

	inline unsigned int to_value(const char *str, unsigned int &value) {
		return (value = (unsigned int) std::stoul(str));
	}

	inline short to_value(const char *str, short &value) {
		return (value = (short) std::stoi(str));
	}

	inline unsigned short to_value(const char *str, unsigned short &value) {
		return (value = (unsigned short) std::stoi(str));
	}

	inline long to_value(const char *str, long &value) {
		return (value = std::stol(str));
	}

	inline unsigned long to_value(const char *str, unsigned long &value) {
		return (value = std::stoul(str));
	}

	inline long long to_value(const char *str, long long &value) {
		return (value = std::stoll(str));
	}

	inline unsigned long long to_value(const char *str, unsigned long long &value) {
		return (value = std::stoull(str));
	}

	inline float to_value(const char *str, float &value) {
		return (value = std::stof(str));
	}

	inline double to_value(const char *str, double &value) {
		return (value = std::stod(str));
	}

	inline int to_value(const pugi::xml_node &node, const int value) {
		return node.attribute("value").as_int(value);
	}

	inline unsigned int to_value(const pugi::xml_node &node, const unsigned int value) {
		return node.attribute("value").as_uint(value);
	}

	inline unsigned short to_value(const pugi::xml_node &node, const unsigned short value) {
		return (unsigned short) node.attribute("value").as_int(value);
	}

	inline float to_value(const pugi::xml_node &node, const float value) {
		return node.attribute("value").as_float(value);
	}

	inline double to_value(const pugi::xml_node &node, const double value) {
		return node.attribute("value").as_double(value);
	}

	inline unsigned long to_value(const pugi::xml_node &node, const unsigned long value) {
		return (unsigned long) node.attribute("value").as_uint(value);
	}

	inline long to_value(const pugi::xml_node &node, const long value) {
		return (long) node.attribute("value").as_int(value);
	}

 }

