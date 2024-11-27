/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/value.h>
 #include <stdexcept>
 #include <string.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/string.h>
 #include <udjat/agent/level.h>
 #include <map>

 using namespace std;

 namespace Udjat {

	Value & Value::set(const char *value, const Type type) {

		clear(type);

		if(!(value && *value)) {
			return *this;
		}

		switch(type) {
		case Undefined:
			break;

		case Array:
			throw logic_error("Cant set array from string");

		case Object:
			throw logic_error("Cant set object from string");

		case String:
		case Icon:
		case Url:
			content.ptr = strdup(value);
			break;

		case Timestamp:
			content.timestamp = TimeStamp::parse(value);
			break;

		case Signed:
			content.sig = stoi(value);
			break;

		case Unsigned:
			content.unsig = (unsigned int) stoul(value);
			break;

		case Real:
		case Fraction:
			content.dbl = stod(value);
			break;

		case Boolean:
			content.sig = Udjat::String{value}.as_bool();
			break;

		case State:
			content.unsig = (unsigned int) LevelFactory(value);
			break;

		default:
			throw logic_error("The value type to set is unexpected or invalid");
		}

		return *this;
	}

	Value & Value::append(Value::Type item_type) {
		if(type == Undefined) {
			clear(Array);
		}

		if(type != Array) {
			throw logic_error("The value is not an array");
		}

		if(!content.ptr) {
			throw runtime_error("Invalid object");
		}

		vector<Value> *children = ((vector<Value> *) content.ptr);

		children->emplace_back(item_type);
		return children->back();

	}

	Value & Value::merge(const Value &src) {

		if(type == Undefined) {
			clear(Object);
		}

		if(src.type != Object || type != Object) {
			throw runtime_error("Merging is only allowed for objects");
		}

		for(const auto & [key, value] : *(( map<std::string,Value> *) src.content.ptr))	{
			(*this)[key.c_str()].set(value);
		}

		return *this;
	}

	Value & Value::set(const Value &src) {

		reset(src.type);
		switch(src.type) {
		case Value::Undefined:
			break;

		case Value::Object:
			merge(src);
			break;

		case Value::String:
		case Value::Icon:
		case Value::Url:
			content.ptr = strdup((const char *) src.content.ptr);
			break;

		case Value::Timestamp:
			content.timestamp = src.content.timestamp;
			break;

		case Value::Signed:
		case Value::Boolean:
			content.sig = src.content.sig;
			break;

		case Value::Unsigned:
		case Value::State:
			content.unsig = src.content.unsig;
			break;

		case Value::Real:
		case Value::Fraction:
			content.dbl = src.content.dbl;
			break;

		default:
			throw runtime_error("Invalid value type");
		}

		return *this;
	}

	Value & Value::setFraction(const float fraction) {
		reset(Fraction);
		content.dbl = fraction;
		return *this;
	}

	Value & Value::set(const short value) {
		reset(Signed);
		content.sig = (int) value;
		return *this;
	}

	Value & Value::set(const unsigned short value) {
		reset(Unsigned);
		content.unsig = (unsigned int) value;
		return *this;
	}

	Value & Value::set(const int value) {
		reset(Signed);
		content.sig = value;
		return *this;
	}

	Value & Value::set(const unsigned int value) {
		reset(Unsigned);
		content.unsig = value;
		return *this;
	}

	Value & Value::set(const TimeStamp &value) {
		reset(Timestamp);
		content.timestamp = value;
		return *this;
	}

	Value & Value::set(const bool value) {
		reset(Boolean);
		content.sig = value;
		return *this;
	}

	Value & Value::set(const float value) {
		reset(Real);
		content.dbl = (double) value;
		return *this;
	}

	Value & Value::set(const double value) {
		reset(Real);
		content.dbl = value;
		return *this;
	}

 }