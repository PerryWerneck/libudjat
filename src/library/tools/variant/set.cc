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
 #include <udjat/tools/variant.h>
 #include <stdexcept>
 #include <string.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/agent/level.h>
 #include <map>
 #include <udjat/tools/string.h>
 #include <private/variant.h>

 using namespace std;

 namespace Udjat {

	Value & Variant::assign(const char *value, const Type type) {

		clear(type);

		if(!(value && *value)) {
			return *this;
		}

		switch(type) {
		case Undefined:
			throw logic_error(Udjat::String{"Unable to set('",value,"') on undefined variant"});
			break;

		case Array:
			throw logic_error("Cant set array from string");

		case ValueMap:
			throw logic_error("Cant set value map from string");

		case DataTable:
			throw logic_error("Cant set data table from string");

		case String:
		case Icon:
		case Url:
		case ObjectPath:
			if(content.ptr) {
				free(content.ptr);
			}
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

	Value & Variant::erase(const char *name) {

		if(type != ValueMap) {
			throw logic_error(Logger::String{"Unable to erase element into a value type '",std::to_string(type),"'"});
		}

		if(content.ptr) {
			(*((map<std::string,Value> *) content.ptr)).erase(name);
		}
			
		return *this;

	}

	Variant::Content & Variant::append_type(const Variant::Type type) {

		if(this->type == Undefined) {
			clear(Array);
		}

		if(!content.ptr) {
			throw runtime_error("Invalid variant");
		}

		if(this->type == Array) {

			// It's an array.
			vector<Value> *children = ((vector<Value> *) content.ptr);
			children->emplace_back(type);
			return children->back().content;

		}

		if(this->type == DataTable) {

			// It's a datatable
			return ((Variant::Table *) content.ptr)->append_type(type);

		}

		throw logic_error("The variant doesn't contain an array.");

	}

	Variant & Variant::append(const char *value, const Variant::Type type) {

		Content &content = append_type(type);

		switch(type) {
		case Variant::Undefined:
			break;

		case Variant::String:
		case Variant::Icon:
		case Variant::Url:
			content.ptr = strdup(value);
			break;

		case Variant::Timestamp:
			content.timestamp = (time_t) TimeStamp{value};
			break;

		case Variant::Signed:
		case Variant::Boolean:
			content.sig = atoi(value);
			break;

		case Variant::Unsigned:
		case Variant::State:
			content.unsig = (unsigned int) atoi(value);
			break;

		case Variant::SignedLong:
			content.sl = atol(value);
			break;

		case Variant::UnsignedLong:
			content.ul = (unsigned long) atol(value);
			break;

		case Variant::Real:
		case Variant::Fraction:
			content.dbl = (double) atof(value);
			break;

		default:
			throw runtime_error("Invalid variant type");

		}

		return *this;
	}

	Variant & Variant::merge(const Variant &src) {

		if(type == Undefined) {
			clear(ValueMap);
		}

		if(src.type == Undefined) {
			return *this;
		}
		
		if(src.type != ValueMap || type != ValueMap) {
			throw runtime_error(
				Logger::Message(
					"Unable to merge '{}' into '{}'",
						std::to_string(src.type),
						std::to_string(type)
				)
			);
		}

#if __cplusplus >= 201703L
        for(const auto & [key, value] : *(( map<std::string,Value> *) src.content.ptr))	{
			(*this)[key.c_str()].assign(value);
		}
#else
                throw system_error(ENOTSUP,system_category(),"Unable to merge values");
#endif

		return *this;
	}

	Value & Variant::assign(const Value &src) {

		reset(src.type);
		switch(src.type) {
		case Variant::Undefined:
			break;

		case Variant::ValueMap:
			merge(src);
			break;

		case Variant::String:
		case Variant::Icon:
		case Variant::Url:
			if(src.content.ptr) {
				content.ptr = strdup((const char *) src.content.ptr);
			} else {
				content.ptr = nullptr;
			}
			break;

		case Variant::Timestamp:
			content.timestamp = src.content.timestamp;
			break;

		case Variant::Signed:
		case Variant::Boolean:
			content.sig = src.content.sig;
			break;

		case Variant::Unsigned:
		case Variant::State:
			content.unsig = src.content.unsig;
			break;

		case Variant::Real:
		case Variant::Fraction:
			content.dbl = src.content.dbl;
			break;

		default:
			throw runtime_error("Invalid variant type");
		}

		return *this;
	}

	Value & Variant::setFraction(const float fraction) {
		reset(Fraction);
		content.dbl = fraction;
		return *this;
	}

 }
