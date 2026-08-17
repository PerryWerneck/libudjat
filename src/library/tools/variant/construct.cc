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
 #include <private/variant.h>
 #include <udjat/tools/variant.h>
 #include <map>
 #include <vector>
 #include <string.h>

 using namespace std;

 namespace Udjat {
 
	Variant::Variant(const Value &src) : Variant{} {

		type = src.type;

		switch(type) {
		case Undefined:
			break;

		case String:
		case Icon:
		case Url:
		case ObjectPath:
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

		case ValueMap:
			if(src.content.ptr) {
				content.ptr = (void *) new map<std::string,Value>(*(( map<std::string,Value> *) src.content.ptr));
			} else {
				content.ptr = (void *) new map<std::string,Value>();
			}
			break;

		case DataTable:
			throw system_error(ENOTSUP,system_category(),"Data table engine is incomplete");
			break;

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

	Variant::Variant(Type type) : Value{} {
		clear(type);
	}

	Variant::~Variant() {
		clear();
	}

	bool Variant::operator==(const char *str) const {
		return isString() && strcasecmp((const char *) content.ptr,str) == 0;
	}

	Variant & Variant::clear(const Type new_type) {

		clear(this->type,this->content);
		type = new_type;

		switch(type) {
		case Undefined:
		case String:
		case Icon:
		case Url:
		case ObjectPath:
			content.ptr = nullptr;
			break;

		case DataTable:
			content.ptr = (void *) new Variant::Table();
			break;

		case Array:
			content.ptr = (void *) new vector<Value>();
			break;

		case ValueMap:
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

	void Variant::clear(const Type type, Content &content) {

		if(content.ptr) {
			if(isString(type)) {
				free(content.ptr);
				content.ptr = nullptr;
			} else if(type == Array) {
				delete ((vector<Variant> *) content.ptr);
			} else if(type == ValueMap) {
				delete ((map<std::string,Variant> *) content.ptr);
			} else if(type == DataTable) {
				delete ((Variant::Table *) content.ptr);
			}
			content.ptr = nullptr;
		}

	}

 }