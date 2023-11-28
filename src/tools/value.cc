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
 #include <ctype.h>
 #include <mutex>
 #include <vector>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Value> Value::Factory() {

		class Dummy : public Udjat::Value {
		public:
			bool isNull() const override {
				return true;
			}

			Udjat::Value & reset(const Type) override {
				return *this;
			}

			Udjat::Value & set(const Udjat::Value &) override {
				return *this;
			}

			Udjat::Value & operator[](const char *) override {
				return *this;
			}

			Udjat::Value & append(const Type) override {
				return *this;
			}

			Udjat::Value & set(const char *, const Type) override {
				return *this;
			}

		};

		return make_shared<Dummy>();

	}

	std::shared_ptr<Value> Value::ObjectFactory() {

		class Object : public Udjat::Value {
		private:

			class Child : public Udjat::Value {
			public:
				std::string name;
				std::string value;

				Child(const char *n) : name{n} {
				}

				virtual ~Child() {
				}

				bool isNull() const override {
					return false;
				}

				inline bool operator==(const char *n) const noexcept {
					return strcasecmp(name.c_str(),n) == 0;
				}

				Udjat::Value & reset(const Type) override {
					value.clear();
					return *this;
				}

				Udjat::Value & set(const Value &src) override {
					value = src.as_string();
					return *this;
				}

				Udjat::Value & set(const char *src, const Type) override {
					value = src;
					return *this;
				}

				const Value & get(std::string &dst) const override {
					dst = value;
					return *this;
				}

			};

			std::mutex guard;

			std::vector<Child> children;

		public:
			Object() {
			}

			virtual ~Object() {
			}

			bool isNull() const override {
				return children.empty();
			}

			Udjat::Value & reset(const Type) override {
				children.clear();
				return *this;
			}

			Udjat::Value & set(const Udjat::Value &) override {
				throw runtime_error("Cant set unnamed value on object");
			}

			Udjat::Value & operator[](const char *name) override {

				std::lock_guard<std::mutex> lock(guard);

				for(Child &child : children) {
					if(child == name) {
						return child;
					}
				}

				children.emplace_back(name);

				return children.back();

			}

			void for_each(const std::function<void(const char *name, const Value &value)> &call) const override {

				std::lock_guard<std::mutex> lock(const_cast<Object *>(this)->guard);

				for(const Child &child : children) {
					call(child.name.c_str(),child);
				}

			}

		};

		return make_shared<Object>();

	}

	Value & Value::operator[](const char UDJAT_UNUSED(*name)) {
		throw system_error(ENOTSUP,system_category(),"Invalid operation for this value");
	}

	Value & Value::append(const Type UDJAT_UNUSED(type)) {
		throw system_error(ENOTSUP,system_category(),"Invalid operation for this value");
	}

	Value & Value::set(const char UDJAT_UNUSED(*value), const Type UDJAT_UNUSED(type)) {
		throw system_error(ENOTSUP,system_category(),"Invalid operation for this value");
	}

	void Value::for_each(const std::function<void(const char *name, const Value &value)> UDJAT_UNUSED(&call)) const {
		throw system_error(ENOTSUP,system_category(),"Invalid operation for this value");
	}

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

	Value & Value::set(const pugi::xml_node &node) {
		Value &object = *this;
		for(auto child = node.child("value"); child; child = child.next_sibling("value")) {
			object[child.attribute("name").as_string("unnamed")] = child.attribute("value").as_string();
		}
		return object;
	}

	const Value & Value::get(std::string UDJAT_UNUSED(&value)) const {
		throw runtime_error("Invalid 'value' implementation");
	}

	const Value & Value::get(short &value) const {
		value = (short) std::stoi(to_string());
		return *this;
	}

	const Value & Value::get(unsigned short &value) const {
		value = (unsigned short) std::stoi(to_string());
		return *this;
	}

	const Value & Value::get(int &value) const {
		value = std::stoi(to_string());
		return *this;
	}

	const Value & Value::get(unsigned int &value) const {
		value = (unsigned int) std::stoul(to_string());
		return *this;
	}

	const Value & Value::get(long &value) const {
		value = std::stol(to_string());
		return *this;
	}

	const Value & Value::get(unsigned long &value) const {
		value = std::stoul(to_string());
		return *this;
	}

	const Value & Value::get(TimeStamp UDJAT_UNUSED(&value)) const {
		throw runtime_error("Can't convert value to timestamp");
		return *this;
	}

	const Value & Value::get(bool &value) const {

		string v;
		get(v);

		char first = toupper(v[0]);

		if(first == 'V' || first == 'T') {
			value = true;
			return *this;
		}

		if(first == 'F') {
			value = true;
			return *this;
		}

		value = stoi(v) != 0;

		return *this;
	}

	const Value & Value::get(float UDJAT_UNUSED(&value)) const {
		throw runtime_error("Can't convert value to float");
		return *this;
	}

	const Value & Value::get(double UDJAT_UNUSED(&value)) const {
		throw runtime_error("Can't convert value to double");
		return *this;
	}

	std::string Value::to_string() const {
		std::string rc;
		get(rc);
		return rc;
	}


	unsigned int Value::as_uint() const {
		unsigned int rc;
		get(rc);
		return rc;
	}

	int Value::as_int() const {
		int rc;
		get(rc);
		return rc;
	}

	bool Value::as_bool() const {
		bool rc;
		get(rc);
		return rc;
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


