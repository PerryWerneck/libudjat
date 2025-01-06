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
	UDJAT_API bool from_string<bool>(const char *str);

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

	template <typename T>
	inline T from_xml(const XML::Node &node, const T def, const char *attrname = "value") {
		throw std::logic_error("No XML converter for this data format");
	}

	template <>
	inline int from_xml<int>(const XML::Node &node, const int def, const char *attrname) {
		return node.attribute(attrname).as_int(def);
	}

	template <>
	inline unsigned int from_xml<unsigned int>(const XML::Node &node, const unsigned int def, const char *attrname) {
		return node.attribute(attrname).as_uint(def);
	}

	template <>
	inline short from_xml<short>(const XML::Node &node, const short def, const char *attrname) {
		return (short) node.attribute(attrname).as_int(def);
	}

	template <>
	inline unsigned short from_xml<unsigned short>(const XML::Node &node, const unsigned short def, const char *attrname) {
		return (unsigned short) node.attribute(attrname).as_int(def);
	}

	template <>
	inline long from_xml<long>(const XML::Node &node, const long def, const char *attrname) {
		return (long) node.attribute(attrname).as_int(def);
	}

	template <>
	inline unsigned long from_xml<unsigned long>(const XML::Node &node, const unsigned long def, const char *attrname) {
		return (unsigned long) node.attribute(attrname).as_uint(def);
	}

	template <>
	inline float from_xml<float>(const XML::Node &node, const float def, const char *attrname) {
		return node.attribute(attrname).as_float(def);
	}

	template <>
	inline double from_xml<double>(const XML::Node &node, const double def, const char *attrname) {
		return node.attribute(attrname).as_double(def);
	}

 }

