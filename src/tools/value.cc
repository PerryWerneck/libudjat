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

 #include <udjat/tools/value.h>
 #include <string>
 #include <sstream>
 #include <iomanip>

 using namespace std;

 namespace Udjat {

	Value & Value::setFraction(const float fraction) {
		std::stringstream out;
		out << std::fixed << std::setprecision(2) << (fraction *100);
		return set(out.str(),Value::Fraction);
	}

	Value & Value::set(const string &value, const Type type) {
		return set(value.c_str(),type);
	}

	Value & Value::set(const short value) {
		return set(std::to_string(value), Signed);
	}

	Value & Value::set(const unsigned short value) {
		return set(std::to_string(value), Unsigned);
	}

	Value & Value::set(const int value) {
		return set(std::to_string(value), Signed);
	}

	Value & Value::set(const unsigned int value) {
		return set(std::to_string(value), Unsigned);
	}

	Value & Value::set(const long value) {
		return set(std::to_string(value), Signed);
	}

	Value & Value::set(const unsigned long value) {
		return set(std::to_string(value), Unsigned);
	}

	Value & Value::set(const TimeStamp value) {
		if(value) {
			return set(value.to_string(), Timestamp);
		}
		return set("", Timestamp);
	}

	Value & Value::set(const bool value) {
		return set(value ? "1" : "0", Boolean);
	}

	Value & Value::set(const float value) {
		return set(std::to_string(value), Real);
	}

	Value & Value::set(const double value) {
		return Value::set(std::to_string(value), Real);
	}

 }

 namespace std {

 	const char * to_string(Udjat::Value::Type type) noexcept {

		static const char *typenames[] = {
			"undefined",
			"array",
			"object",
			"string",
			"timestamp",
			"signed",
			"unsigned",
			"real",
			"boolean",
			"fraction"
		};

		if( (size_t) type >= (N_ELEMENTS(typenames)) )
			return "unknown";

		return typenames[type];

 	}

 }


