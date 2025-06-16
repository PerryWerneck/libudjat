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
 #include <udjat/tools/logger.h>
 #include <stdexcept>
 #include <udjat/agent/level.h>
 #include <udjat/tools/string.h>
 #include <cstdlib>
 #include <map>
 #include <vector>
 #include <functional>
 #include <stdexcept>
 #include <sstream>
 #include <iomanip>

 using namespace std;

 namespace Udjat {
 
 	bool Value::empty() const noexcept {

		if(type == Array) {

			if(!content.ptr) {
				return true;
			}
			return ((vector<Value> *) content.ptr)->empty();

		} else if(type == Object) {
			if(!content.ptr) {
				return true;
			}
			return (((map<std::string,Value> *) content.ptr))->empty();

		} else if(type == String) {

			return !(content.ptr && *((char *) content.ptr)); 

		} else if(type == Undefined) {

			return true;

		}

		return false;

	}

	bool Value::isNull() const noexcept {
		return (type == Undefined) || ((type == String || type == Icon || type == Url) && !content.ptr);
	}

	bool Value::isString() const noexcept {
		return (type == String || type == Icon || type == Url) && content.ptr;
	}

	const char * Value::c_str() const noexcept {
		if(isString()) {
			return (const char *) content.ptr;
		}
		return "";
	}

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
			if(content.ptr) {
				value = (const char *) content.ptr;
			} else {
				value.clear();
			}
			break;

		case Timestamp:
			value = TimeStamp{content.timestamp}.to_string(TIMESTAMP_FORMAT_JSON);
			break;

		case Signed:
			value = std::to_string(content.sig);
			break;

		case Unsigned:
			value = std::to_string(content.unsig);
			break;

		case Real:
			{
				std::stringstream out;
				out.imbue(std::locale("C"));
				out << std::fixed << std::setprecision(2) << content.dbl;
				value = out.str();
			}
			break;

		case Fraction:
			{
				std::stringstream out;
				out.imbue(std::locale("C"));
				out << std::fixed << std::setprecision(2) << (content.dbl *100) << "%";
				value = out.str();
			}
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

	class Value::Getter {
	public:

#if __cplusplus >= 201703L	
		const Value &src;

		constexpr Getter(const Value &value) : src{value} {
		}

		template <typename T>
		inline const Value & get(T &dst) const {

			switch((Value::Type) src) {
			case Value::Undefined:
				throw logic_error("The value is undefined");
				break;

			case Value::Array:
			case Value::Object:
				throw logic_error("Unable to convert value");
				break;

			case Value::Icon:
			case Value::Url:
			case Value::String:
				dst = (T) stoi((const char *) src.content.ptr);
				break;

			case Value::Timestamp:
				dst = (T) src.content.timestamp;
				break;

			case Value::Signed:
			case Value::Boolean:
				dst = (T) src.content.sig;
				break;

			case Value::Unsigned:
			case Value::State:
				dst = (T) src.content.unsig;
				break;

			case Value::Real:
			case Value::Fraction:
				dst = (T) src.content.dbl;
				break;

			default:
				throw logic_error("The value type to get is unexpected or invalid");
			}

			return src;
		}

#else
		const Value *src;

		Getter(const Value &value) : src{&value} {
		}

		template <typename T>
		inline const Value & get(T &dst) const {

			switch((Value::Type) *src) {
			case Value::Undefined:
				throw logic_error("The value is undefined");
				break;

			case Value::Array:
			case Value::Object:
				throw logic_error("Unable to convert value");
				break;

			case Value::Icon:
			case Value::Url:
			case Value::String:
				dst = (T) stoi((const char *) src->content.ptr);
				break;

			case Value::Timestamp:
				dst = (T) src->content.timestamp;
				break;

			case Value::Signed:
			case Value::Boolean:
				dst = (T) src->content.sig;
				break;

			case Value::Unsigned:
			case Value::State:
				dst = (T) src->content.unsig;
				break;

			case Value::Real:
			case Value::Fraction:
				dst = (T) src->content.dbl;
				break;

			default:
				throw logic_error("The value type to get is unexpected or invalid");
			}

			return src;
		}

#endif

	};

	const Value & Value::get(short &value) const {
		if(type == String) {
			value = (short) atoi((const char *) content.ptr);
			return *this;
		}
		return Getter{*this}.get(value);
	}

	const Value & Value::get(unsigned short &value) const {
		if(type == String) {
			value = (unsigned short) atoi((const char *) content.ptr);
			return *this;
		}
		return Getter{*this}.get(value);
	}

	const Value & Value::get(int &value) const {
		if(type == String) {
			value = (int) atoi((const char *) content.ptr);
			return *this;
		}
		return Getter{*this}.get(value);
	}

	const Value & Value::get(unsigned int &value) const {
		if(type == String) {
			value = (unsigned int) atol((const char *) content.ptr);
			return *this;
		}
		return Getter{*this}.get(value);
	}

	const Value & Value::get(long &value) const {
		if(type == String) {
			value = (long) atol((const char *) content.ptr);
			return *this;
		}
		return Getter{*this}.get(value);
	}

	const Value & Value::get(unsigned long &value) const {
		if(type == String) {
			value = (unsigned long) atol((const char *) content.ptr);
			return *this;
		}
		return Getter{*this}.get(value);
	}

	const Value & Value::get(TimeStamp &value) const {
		if(likely(type == Timestamp)) {
			value = TimeStamp{content.timestamp};
		} else if(type == String) {
			value = TimeStamp{(const char *) content.ptr};
		} else {
			throw logic_error("The value doesnt contains a timestamp");
		}
		return *this;
	}

	bool Value::as_bool() const {
		bool rc;
		get(rc);
		return rc;
	}

	const Value & Value::get(bool &value) const {
		if(type == String) {
			value = Udjat::String{(const char *) content.ptr}.as_bool();
			return *this;
		}
		return Getter{*this}.get(value);
	}

	const Value & Value::get(float &value) const {
		if(type == String) {
			value = atof((const char *) content.ptr);
			return *this;
		}
		return Getter{*this}.get(value);
	}

	const Value & Value::get(double &value) const {
		if(type == String) {
			value = atof((const char *) content.ptr);
			return *this;
		}
		return Getter{*this}.get(value);
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
		
		if(type == Undefined) {
			clear(Array);
		}

		if(type == Array) {

			return ((vector<Value> *) content.ptr)->at(ix);

		} else if(type == Object) {

#if __cplusplus >= 201703L
			for(auto & [key, value] : *((map<std::string,Value> *) content.ptr)) {
				if(ix-- <= 0) {
					return value;
				}
			}
			throw out_of_range("out of range");
#else
			throw system_error(ENOTSUP,system_category(),"Not implemented in legacy mode");
#endif

		} else if(ix == 0) {

			return *this;

		}
	
		throw out_of_range("The value is not an array");

	}

	const Value & Value::operator[](int ix) const {

		if(type == Array) {

			return ((const vector<Value> *) content.ptr)->at(ix);

		} else if(type == Object) {
#if __cplusplus >= 201703L
			for(const auto & [key, value] : *((map<std::string,Value> *) content.ptr)) {
				if(ix-- <= 0) {
					return value;
				}
			}
			throw out_of_range("out of range");
#else
			throw system_error(ENOTSUP,system_category(),"Not implemented in legacy mode");
#endif

		} else if(ix == 0) {

			return *this;

		}
	
		throw out_of_range("The value is not an array");
	}

	Value & Value::append(const char *name, Value::Type type) {
		
		if(type == Undefined) {
			clear(Object);
		}

		if(type != Object) {
			throw logic_error(Logger::String{"Unable to append element into a value type '",std::to_string(type),"'"});
		}
			
		return (*((map<std::string,Value> *) content.ptr))[name].clear(type);

	}

	bool Value::contains(const char *name) const noexcept {

		if(type != Object) {
			return false;
		}

		const map<std::string,Value> &children = *((map<std::string,Value> *) content.ptr);
		return children.find(name) != children.end();

	}

	Value & Value::operator[](const char *name) {

		if(type == Undefined) {
			clear(Object);
		}

		if(type == Object) {
			return (*((map<std::string,Value> *) content.ptr))[name];
		}

		throw logic_error(Logger::String{"Unable to get children from a value type '",std::to_string(type),"'"});
	}

	bool Value::getProperty(const char *key, std::string &value) const {
		if(type == Object && content.ptr) {
			const map<std::string,Value> &children = *((map<std::string,Value> *) content.ptr);
			auto it = children.find(key);
			if(it == children.end()) {
				return false;
			}
			value = it->second.to_string();
			return true;
		}
		return Object::getProperty(key,value);
	}

	const Value & Value::operator[](const char *name) const {

		if(type != Object) {
			throw runtime_error(Logger::String{"Cant get child '",name,"': Value is not an object"});
		}

		return ((map<std::string,Value> *) content.ptr)->at(name);
		
	}

	bool Value::for_each(const std::function<bool(const char *name, const Value &value)> &call) const {

		if(type == Array) {
			if(!content.ptr) {
				return *this;
			}
			size_t ix = 0;
			for(const auto &item : *((vector<Value> *) content.ptr)) {
				if(call(std::to_string(ix++).c_str(),item)) {
					return true;
				}
			}
		} else if(type == Object) {
			if(!content.ptr) {
				return *this;
			}
#if __cplusplus >= 201703L
			for(const auto & [key, value] : *((map<std::string,Value> *) content.ptr)) {
				if(call(key.c_str(),value)) {
					return true;
	                	}
			}
#else
			throw system_error(ENOTSUP,system_category(),"Not implemented in legacy mode");
#endif
		} else if(type != Undefined) {
			return call("",*this);
		}

		return false;

	}

	bool Value::for_each(const std::function<bool(const Value &value)> &call) const {

		if(type == Array) {
			if(!content.ptr) {
				return *this;
			}
			for(const auto &item : *((vector<Value> *) content.ptr)) {
				if(call(item)) {
					return true;
				}
			}
		} else if(type == Object) {
			if(!content.ptr) {
				return *this;
			}
#if __cplusplus >= 201703L
			for(const auto & [key, value] : *((map<std::string,Value> *) content.ptr)) {
				if(call(value)) {
                			return true;
                		}
			}
#else
			throw system_error(ENOTSUP,system_category(),"Not implemented in legacy mode");
#endif
		} else if(type != Undefined) {
			return call(*this);
		}

		return false;

	}

	std::string Value::serialize(const MimeType mimetype) const {
		stringstream stream;
		serialize(stream,mimetype);
		return stream.str();
	}

	void Value::serialize(std::ostream &out, const MimeType mimetype) const {

		debug("Serializing value");

		switch(mimetype) {
		case MimeType::html:
			to_html(out);
			break;

		case MimeType::json:
			to_json(out);
			break;

		case MimeType::xml:
			to_xml(out);
			break;

		case MimeType::yaml:
			to_yaml(out);
			break;

		case MimeType::sh:
			to_sh(out);
			break;

		default:
			throw runtime_error(Logger::String{"Unable to serialize value to ",std::to_string(mimetype)});
		}

	}

	std::string Value::to_string(const MimeType mimetype) const {
		stringstream stream;
		serialize(stream,mimetype);
		return stream.str();
	}

	std::string Value::to_string() const noexcept {
		string value;
		get(value);
		return value;
	}

	std::string Value::to_string(const char *def) const {
		if(type == Undefined || type == Array || type == Object) {
			return def;
		}
		return to_string();
	}

 }
