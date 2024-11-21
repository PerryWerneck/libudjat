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
 #include <udjat/tools/value.h>
 #include <udjat/tools/report.h>
 #include <map>
 #include <vector>
 #include <string.h>

 using namespace std;

 namespace Udjat {
 
	Value::Value(const Value &src) : Value{} {

		type = src.type;

		switch(type) {
		case Undefined:
			break;

		case String:
		case Icon:
		case Url:
			if(src.content.ptr) {
				content.ptr = strdup((const char *) src.content.ptr);
			} else {
				content.ptr = nullptr;
			}
			break;

		case Array:
			if(src.content.ptr) {
				content.ptr = (void *) new vector<Value>(*((vector<Value> *) src.content.ptr));
			} else {
				content.ptr = (void *) new vector<Value>();
			}
			break;

		case Object:
			if(src.content.ptr) {
				content.ptr = (void *) new map<std::string,Value>(*(( map<std::string,Value> *) src.content.ptr));
			} else {
				content.ptr = (void *) new map<std::string,Value>();
			}
			break;

		case Report:
			throw runtime_error("Cant copy report");

		case Timestamp:
			content.timestamp = src.content.timestamp;
			break;

		case Signed:
		case Boolean:
			content.sig = src.content.sig;
			break;

		case Unsigned:
		case State:
			content.unsig = src.content.unsig;
			break;

		case Real:
		case Fraction:
			content.dbl = src.content.dbl;
			break;

		}

	}

	Value::Value(Type type) : Value{} {
		clear(type);
	}

	Value::~Value() {
		clear();
	}

	bool Value::operator==(const char *str) const {
		return isString() && strcasecmp((const char *) content.ptr,str) == 0;
	}

	Value & Value::clear(const Type new_type) {

		if(content.ptr) {
			if(type == String || type == Url || type == Icon) {
				free(content.ptr);
			} else if(type == Array) {
				delete ((vector<Value> *) content.ptr);
			} else if(type == Object) {
				delete ((map<std::string,Value> *) content.ptr);
			} else if(type == Report) {
				delete ((Udjat::Report *) content.ptr);
			}
			content.ptr = nullptr;
		}

		type = new_type;

		switch(type) {
		case Report:
			throw logic_error("Unable to set value type to reserved value");
			
		case Undefined:
		case String:
		case Icon:
		case Url:
			content.ptr = nullptr;
			break;

		case Array:
			content.ptr = (void *) new vector<Value>();
			break;

		case Object:
			content.ptr = (void *) new map<std::string,Value>();
			break;

		case Timestamp:
			content.timestamp = 0;
			break;

		case Signed:
		case Boolean:
			content.sig = 0;
			break;

		case Unsigned:
		case State:
			content.unsig = 0;
			break;

		case Real:
		case Fraction:
			content.dbl = 0;
			break;

		}

		return *this;

	}

	/// @brief Convert this value to report.
	/// @return The report handler.
	Udjat::Report & Value::ReportFactory(const char *column_name, ... ) {

		clear();

		va_list args;
		va_start(args, column_name);
		Udjat::Report *worker = new Udjat::Report(column_name,args);
		va_end(args);		

		type = Report;
		content.ptr = (void *) worker;

		return *worker;

	}

	/// @brief Add report node to object value.
	/// @return The report handler.
	Udjat::Report & Value::ReportChildFactory(const char *name, const char *column_name, ... ) {

		Udjat::Value &val = (*this)[name];

		val.clear();

		va_list args;
		va_start(args, column_name);
		Udjat::Report *worker = new Udjat::Report(column_name,args);
		va_end(args);		

		val.type = Report;
		val.content.ptr = (void *) worker;

		return *worker;

	}

 }