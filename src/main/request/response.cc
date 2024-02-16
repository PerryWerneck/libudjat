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
 #include <algorithm>
 #include <private/request.h>
 #include <udjat/tools/abstract/response.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/response/value.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/exception.h>
 #include <ctime>

 using namespace std;

 namespace Udjat {

	void Response::Value::serialize(std::ostream &stream) const {

		Abstract::Response::serialize(stream);

		switch(mimetype) {
		case Udjat::MimeType::xml:

			// Format as XML
			to_xml(stream);
			stream << "</response>";

			break;

		case Udjat::MimeType::json:
			Udjat::Value::serialize(stream,mimetype);
			stream << "}";
			break;

		default:
			Udjat::Value::serialize(stream,mimetype);
		}

	}

	Response::Value::operator Value::Type() const noexcept {
		return Value::Object;
	}

	bool Response::Value::isNull() const {
		return false;
	}

	Udjat::Value & Response::Value::reset(const Udjat::Value::Type) {
		throw system_error(EPERM,system_category(),"Response types are fixed");
	}

	Udjat::Value & Response::Value::set(const Udjat::Value &) {
		throw system_error(EPERM,system_category(),"Response types are fixed");
	}

	std::string Response::Value::to_string() const noexcept {

		try {

			return Udjat::Value::to_string(mimetype);

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(STRINGIZE_VALUE_OF(PRODUCT_NAME));

		}

		return "";
	}

	/*
	std::shared_ptr<Response::Value> Response::Value::Factory(Udjat::MimeType mimetype) {

		class UDJAT_API Value : public Udjat::Response::Value {
		private:

			class Child : public Udjat::Value {
			};

			Value::Type type = Value::Object;
			std::map<std::string,Child> children;

		public:
			Value(Udjat::MimeType mimetype) : Response::Value{mimetype} {
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
				return children[name];
			}

			Udjat::Value & append(const Type type) override {
				reset(Value::Array);
				return children[std::to_string((int) children.size()).c_str()];
			}

			Udjat::Value & reset(const Udjat::Value::Type type) override {
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

			Udjat::Value & set(const char *value, const Type type = String) override {
			}

		}

		return make_shared<Value>(mimetype);
	}
	*/

 }

