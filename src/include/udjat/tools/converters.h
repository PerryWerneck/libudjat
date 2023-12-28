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
 #include <stdexcept>
 #include <udjat/tools/xml.h>

 namespace Udjat {

	template <typename T>
	inline T from_string(const char *str) {
		throw std::logic_error("No converter for this data format");
	}

 	template <>
	inline int from_string<int>(const char *str) {
		return std::stoi(str);
	}

 	template <>
	inline unsigned int from_string<unsigned int>(const char *str) {
		return (unsigned int) std::stoul(str);
	}

 	template <>
	inline short from_string<short>(const char *str) {
		return (short) std::stoi(str);
	}

 	template <>
	inline unsigned short from_string<unsigned short>(const char *str) {
		return (unsigned short) std::stoi(str);
	}

 	template <>
	inline long from_string<long>(const char *str) {
		return std::stol(str);
	}

 	template <>
	inline unsigned long from_string<unsigned long>(const char *str) {
		return std::stoul(str);
	}

 	template <>
	inline long long from_string<long long>(const char *str) {
		return std::stoll(str);
	}

 	template <>
	inline unsigned long long from_string<unsigned long long>(const char *str) {
		return std::stoull(str);
	}

 	template <>
	inline float from_string<float>(const char *str) {
		return std::stof(str);
	}

 	template <>
	inline double from_string<double>(const char *str) {
		return std::stod(str);
	}

	inline int to_value(const XML::Node &node, const int value, const char *attrname = "value") {
		return node.attribute(attrname).as_int(value);
	}

	inline unsigned int to_value(const XML::Node &node, const unsigned int value, const char *attrname = "value") {
		return node.attribute(attrname).as_uint(value);
	}

	inline unsigned short to_value(const XML::Node &node, const unsigned short value, const char *attrname = "value") {
		return (unsigned short) node.attribute(attrname).as_int(value);
	}

	inline float to_value(const XML::Node &node, const float value, const char *attrname = "value") {
		return node.attribute(attrname).as_float(value);
	}

	inline double to_value(const XML::Node &node, const double value, const char *attrname = "value") {
		return node.attribute(attrname).as_double(value);
	}

	inline unsigned long to_value(const XML::Node &node, const unsigned long value, const char *attrname = "value") {
		return (unsigned long) node.attribute(attrname).as_uint(value);
	}

	inline long to_value(const XML::Node &node, const long value, const char *attrname = "value") {
		return (long) node.attribute(attrname).as_int(value);
	}

 }

