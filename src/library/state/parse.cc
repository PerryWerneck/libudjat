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

/**
 * @file src/core/state/parse.cc
 *
 * @brief Implements the range parsers
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/state.h>
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <iostream>
 #include <limits.h>
 #include <udjat/tools/parse.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

	/// @brief Parser for range or value attributes.
	struct AttributeParser {

		XML::Attribute from;
		XML::Attribute to;

		AttributeParser(const XML::Node &node) {

			XML::Attribute value = node.attribute("value");

			from = node.attribute("from-value");
			if(!from) {
				from = value;
			}

			to = node.attribute("to-value");
			if(!to) {
				to = value;
			}

		}

	};

	void XML::parse(const XML::Node &node, int &from, int &to) {

		AttributeParser parser{node};

		from = parser.from.as_int(INT_MIN);
		to = parser.to.as_int(INT_MAX);

	}

	void XML::parse(const XML::Node &node, unsigned int &from, unsigned int &to) {

		AttributeParser parser{node};

		from = parser.from.as_uint(0);
		to = parser.to.as_uint(UINT_MAX);

	}

	void XML::parse(const XML::Node &node, unsigned short &from, unsigned short &to) {

		AttributeParser parser{node};

		from = parser.from.as_uint(0);
		to = parser.to.as_uint(USHRT_MAX);

	}

	void XML::parse(const XML::Node &node, float &from, float &to) {

		AttributeParser parser{node};

		from = parser.from.as_float(numeric_limits<float>::min());
		to = parser.to.as_float(numeric_limits<float>::max());

	}

	void XML::parse(const XML::Node &node, double &from, double &to) {

		AttributeParser parser{node};

		from = parser.from.as_double(numeric_limits<double>::min());
		to = parser.to.as_double(numeric_limits<double>::max());

	}

	void XML::parse(const XML::Node &node, unsigned long &from, unsigned long &to) {

		AttributeParser parser{node};

		from = (unsigned long) parser.from.as_ullong();
		to = (unsigned long) parser.from.as_ullong(ULONG_MAX);

	}

	void XML::parse(const XML::Node &node, long &from, long &to) {

		AttributeParser parser{node};

		from = (long) parser.from.as_llong(LONG_MIN);
		to = (long) parser.from.as_llong(LONG_MAX);

	}

	void XML::parse(const XML::Node &node, long long &from, long long &to) {

		AttributeParser parser{node};

		from = (long) parser.from.as_llong(numeric_limits<long long>::min());
		to = (long) parser.from.as_llong(numeric_limits<long long>::max());

	}

	void XML::parse(const XML::Node &node, unsigned long long &from, unsigned long long &to) {

		AttributeParser parser{node};

		from = (long) parser.from.as_ullong(numeric_limits<unsigned long long>::min());
		to = (long) parser.from.as_ullong(numeric_limits<unsigned long long>::max());

	}

	void XML::parse(const XML::Node &node, int &value) {
		value = node.attribute("value").as_int();
	}

	void XML::parse(const XML::Node &node, unsigned int &value) {
		value = node.attribute("value").as_uint();
	}

	void XML::parse(const XML::Node &node, unsigned short &value) {
		value = (unsigned short) node.attribute("value").as_uint();
	}

	void XML::parse(const XML::Node &node, float &value) {
		value = node.attribute("value").as_float();
	}

	void XML::parse(const XML::Node &node, double &value) {
		value = node.attribute("value").as_double();
	}

	void XML::parse(const XML::Node &node, unsigned long &value) {
		value = (unsigned long) node.attribute("value").as_ullong();
	}

	void XML::parse(const XML::Node &node, long &value) {
		value = (long) node.attribute("value").as_llong();
	}

	void parse_byte_range(const XML::Node &node, unsigned long long &from, unsigned long long &to) {

		auto value = node.attribute("value");

		if(value) {
			from = to = String{value.as_string()}.as_ull();
			return;
		}

		auto f = node.attribute("from-value");
		if(f) {
			from = String{f.as_string()}.as_ull();
		} else {
			from  = 0;
		}

		auto t = node.attribute("to-value");
		if(t) {
			from = String{t.as_string()}.as_ull();
		} else {
			to = ULLONG_MAX;
		}

	}

 }
