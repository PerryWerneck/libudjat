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
 #include <udjat/agent/level.h>
 #include <udjat/tools/string.h>
 #include <cstdlib>
 #include <map>
 #include <vector>
 #include <functional>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {
 
	const Value & Value::get(std::string &value) const {

		switch(type) {
		case Undefined:
			value.clear();
			break;

		case Array:
			throw logic_error("Cant copy array to string");

		case Object:
			throw logic_error("Cant copy object to string");

		case String:
		case Icon:
		case Url:
			value = (const char *) content.ptr;
			break;

		case Timestamp:
			value = TimeStamp{content.timestamp}.to_string();
			break;

		case Signed:
			value = std::to_string(content.sig);
			break;

		case Unsigned:
			value = std::to_string(content.unsig);
			break;

		case Real:
			value = std::to_string(content.dbl);
			break;

		case Fraction:
			value = std::to_string(content.dbl);
			break;

		case Boolean:
			value = std::to_string(content.sig);
			break;

		case State:
			value = std::to_string((Udjat::Level) content.unsig);
			break;

		default:
			throw logic_error("The value type to get is unexpected or invalid");
		}

		return *this;
	}

	template <typename T>
	static const Value & convert(const Value &src, T &dst) {

		switch(type) {
		case Undefined:
			throw logic_error("The value is undefined");
			break;

		case Array:
		case Object:
		case Icon:
		case Url:
		case String:
			throw logic_error("Unable to convert value");
			break;

		case String:
			value = (T) stoi((const char *) content.ptr);
			break;

		case Timestamp:
			value = (T) content.timestamp;
			break;

		case Signed:
		case Boolean:
			value = (T) content.sig;
			break;

		case Unsigned:
		case State:
			value = (T) content.unsig;
			break;

		case Real:
		case Fraction:
			value = (T) content.dbl;
			break;

		default:
			throw logic_error("The value type to get is unexpected or invalid");
		}


	}

	const Value & Value::get(short &value) const {
		if(type == String) {
			value = (short) atoi((const char *) content.ptr);
			return *this;
		}
		return convert<short>(*this,value);
	}

	const Value & Value::get(unsigned short &value) const {
		if(type == String) {
			value = (unsigned short) atoi((const char *) content.ptr);
			return *this;
		}
		return convert<unsigned short>(*this,value);
	}

	const Value & Value::get(int &value) const {
		if(type == String) {
			value = (int) atoi((const char *) content.ptr);
			return *this;
		}
		return convert<int>(*this,value);
	}

	const Value & Value::get(unsigned int &value) const {
		if(type == String) {
			value = (unsigned int) atol((const char *) content.ptr);
			return *this;
		}
		return convert<unsigned int>(*this,value);
	}

	const Value & Value::get(long &value) const {
		if(type == String) {
			value = (long) atol((const char *) content.ptr);
			return *this;
		}
		return convert<long>(*this,value);
	}

	const Value & Value::get(unsigned long &value) const {
		if(type == String) {
			value = (unsigned long) atol((const char *) content.ptr);
			return *this;
		}
		return convert<unsigned long>(*this,value);
	}

	const Value & Value::get(TimeStamp &value) const {
		if(type == String) {
			value = TimeStamp((const char *) content.ptr));
		} else {
			unsigned long tm;
			get(tm);
			value = TimeStamp(tm);
		}
		return *this;
	}

	const Value & Value::get(bool &value) const {
		if(type == String) {
			value = Udjat::String{(const char *) content.ptr}.as_bool();
			return *this;
		}
		return convert<bool>(*this,value);
	}

	const Value & Value::get(float &value) const {
		if(type == String) {
			value = atof((const char *) content.ptr);
			return *this;
		}
		return convert<float>(*this,value);
	}

	const Value & Value::get(double &value) const {
		if(type == String) {
			value = atof((const char *) content.ptr);
			return *this;
		}
		return convert<double>(*this,value);
	}

	size_t Value::size() const {

		if(type == Array) {
			return ((vector<Value> *) content.ptr)->size();
		} else if(type == Object) {
			return (((map<std::string,Value> *) content.ptr))->size();
		} else if(type == Undefined) {
			return 0;
		}

		return 1;
	}

	Value & Value::operator[](int ix) {
		
		if(type == Array) {

			return ((vector<Value> *) content.ptr)->at(ix);

		} else if(type == Object) {

			for(auto & [key, value] : *((map<std::string,Value> *) content.ptr)) {
				if(ix-- <= 0) {
					return value;
				}
			}
			throw out_of_range("out of range");

		} else if(ix == 0) {

			return *this;

		}
	
		throw out_of_range("The value is not an array");
		
	}

	const Value & Value::operator[](int ix) const {
		
		if(type == Array) {

			return ((const vector<Value> *) content.ptr)->at(ix);

		} else if(type == Object) {

			for(const auto & [key, value] : *((map<std::string,Value> *) content.ptr)) {
				if(ix-- <= 0) {
					return value;
				}
			}
			throw out_of_range("out of range");

		} else if(ix == 0) {

			return *this;

		}
	
		throw out_of_range("The value is not an array");
	}

	Value & Value::operator[](const char *name) {
		if(type == Object) {
			return (*((map<std::string,Value> *) content.ptr))[name];
		}
		throw logic_error("The value is not an object");
	}

	bool Value::getProperty(const char *key, std::string &value) const {
		if(type == Object) {
			const map<std::string,Value> &children = *((map<std::string,Value> *) content.ptr);
			auto it = children.find(key);
			if(it == children.end()) {
				return false;
			}
			value = it->second.to_string();
		}
		return false;
	}

	const Value & Value::operator[](const char *name) const {
		if(type == Object) {
			const map<std::string,Value> &children = *((map<std::string,Value> *) content.ptr);
			auto it = children.find(name);
			if(it != children.end()) {
				return it->second;
			}
			throw runtime_error("Cant find requested value");
		}
		throw logic_error("The value is not an object");
	}

	bool Value::for_each(const std::function<bool(const char *name, const Value &value)> &call) const {

		if(type == Array) {
			size_t ix = 0;
			for(const auto item : *((vector<Value> *) content.ptr)) {
				if(call(std::to_string(ix++).c_str(),item)) {
					return true;
				}
			}
		} else if(type == Object) {
			for(const auto & [key, value] : *((map<std::string,Value> *) content.ptr)) {
				if(call(key.c_str(),value)) {
                	return true;
                }
			}
		} else if(type == Undefined) {
			return false;
		}

		return call("",*this);

	}

	bool Value::for_each(const std::function<bool(const Value &value)> &call) const {

		if(type == Array) {
			for(const auto item : *((vector<Value> *) content.ptr)) {
				if(call(item)) {
					return true;
				}
			}
		} else if(type == Object) {
			for(const auto & [key, value] : *((map<std::string,Value> *) content.ptr)) {
				if(call(value)) {
                	return true;
                }
			}
		} else if(type == Undefined) {
			return false;
		}

		return call(*this);

	}

 }