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

 /**
  * @brief Implement response object.
  */

/*
 #include <config.h>
 #include <udjat/tools/abstract/response.h>
 #include <udjat/tools/response/value.h>
 #include <udjat/tools/response/object.h>
 #include <udjat/tools/logger.h>
 #include <map>
 #include <stdexcept>
 #include <iostream>
 #include <iomanip>
 #include <sstream>

 using namespace std;

 namespace Udjat {

 	class Response::Object::Value : public Udjat::Value {
	private:
		Udjat::Value::Type type = Udjat::Value::Undefined;
		std::string value;
		std::map<std::string,Value> children;

	public:

		Value() {
		}

		virtual ~Value() {
		}

		bool empty() const noexcept override {
			return children.empty();
		}

		bool isNull() const override {
			return this->type == Udjat::Value::Undefined;
		}

		operator Type() const noexcept override {
			return this->type;
		}

		bool for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const override {
			for(const auto& [name, value] : children)	{
				if(call(name.c_str(),(Udjat::Value &) value)) {
					return true;
				}
			}
			return false;
		}

		Udjat::Value & operator[](const char *name) override {
			reset(Value::Object);
			return children[name];
		}

		Udjat::Value & append(const Type type) override {
			reset(Value::Array);
			return children[std::to_string((int) children.size()).c_str()];
		}

		Udjat::Value & reset(const Type type) override {
			if(type != this->type) {
				debug("Response reset to '",std::to_string(type),"'");
				this->type = type;
				children.clear();
			}
			return *this;
		}

		Udjat::Value & set(const char *value, const Type type) override {
			reset(type);
			this->value = value;
			return *this;
		}

		const Udjat::Value & get(std::string &value) const override {
			if(!children.empty()) {

				// Has children, check standard names.
				static const char *names[] = {
					"summary",
					"value",
					"name"
				};

				for(size_t ix = 0; ix < (sizeof(names)/sizeof(names[0]));ix++) {

					auto child = children.find(names[ix]);
					if(child != children.end()) {
						child->second.get(value);
						if(!value.empty()) {
							return *this;
						}
					}

				}

			}

			value = this->value;
			return *this;
		}

		Udjat::Value & set(const Udjat::Value &) override {
			throw runtime_error("Not implemented");
		}

		Udjat::Value & set(const Udjat::TimeStamp value) override {
			if(value)
				return this->set(value.to_string(TIMESTAMP_FORMAT_JSON).c_str(),Udjat::Value::String);
			return this->set("false",Value::Type::Boolean);
		}

		Udjat::Value & setFraction(const float fraction) {
			std::stringstream out;
			out.imbue(std::locale("C"));
			out << std::fixed << std::setprecision(2) << (fraction *100);
			return Udjat::Value::set(out.str(),Value::Fraction);
		}

		Udjat::Value & set(const float value) {
			std::stringstream out;
			out.imbue(std::locale("C"));
			out << value;
			return Udjat::Value::set(out.str(),Value::Real);
		}

		Udjat::Value & set(const double value) {
			std::stringstream out;
			out.imbue(std::locale("C"));
			out << value;
			return Udjat::Value::set(out.str(),Value::Real);
		}

 	};

	Response::Object::Object(Udjat::MimeType mimetype) : Udjat::Response::Value{mimetype} {
	}

	Response::Object::~Object() {
	}

	Response::Object::operator Type() const noexcept {
		return this->type;
	}

	bool Response::Object::empty() const noexcept {
		return children.empty();
	}

	bool Response::Object::for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const {
		for(const auto& [name, value] : children)	{
			if(call(name.c_str(),(Udjat::Value &) value)) {
				return true;
			}
		}
		return false;
	}

	Udjat::Value & Response::Object::operator[](const char *name) {
		reset(Value::Object);
		return children[name];
	}

	Udjat::Value & Response::Object::append(const Udjat::Value::Type type) {
		reset(Value::Array);
		return children[std::to_string((int) children.size()).c_str()];
	}

	Udjat::Value & Response::Object::reset(const Udjat::Value::Type type) {
		if(type != Value::Array && type != Value::Object) {
			throw runtime_error(Logger::String{"Cant handle '",std::to_string(type),"' at this level"});
		}

		if(type != this->type) {
			debug("Response reset to '",std::to_string(type),"'");

			this->type = type;
			children.clear();
		}

		return *this;
	}

	Udjat::Value & Response::Object::set(const char *value, const Udjat::Value::Type type) {
		Udjat::Value &child{(*this)["value"]};
		child.set(value,type);
		debug("Response value set to '",child.to_string(),"'");
		return child;
	}

 }


*/
