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
  * @brief Implement response value.
  */

/*
 #include <config.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/logger.h>
 #include <string>
 #include <map>

 using namespace std;

 namespace Udjat {

	Response::Value::Value() {
	}

	Response::Value::~Value() {
	}

	Response::Value::operator Type() const noexcept {
		return this->type;
	}

	bool Response::Value::empty() const noexcept {
		return children.empty();
	}

	bool Response::Value::isNull() const {
		return this->type == Udjat::Value::Undefined;
	}

	bool Response::Value::for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const {
		for(const auto& [name, value] : children)	{
			if(call(name.c_str(),(Udjat::Value &) value)) {
				return true;
			}
		}
		return false;
	}

	Udjat::Value & Response::Value::operator[](const char *name) {
		this->type = Value::Object;
		return children[name];
	}

	Udjat::Value & Response::Value::append(const Udjat::Value::Type) {
		reset(Value::Array);
		return children[std::to_string((int) children.size()).c_str()];
	}

	Udjat::Value & Response::Value::reset(const Udjat::Value::Type type) {
		if(type != this->type) {
			debug("Response reset to '",std::to_string(type),"'");
			this->type = type;
			children.clear();
		}
		return *this;
	}

	Udjat::Value & Response::Value::set(const char *value, const Type type) {
		debug("set(",std::to_string(type),")='",value,"'");
		this->type = type;
		this->value = value;
		return *this;
	}

	const Udjat::Value & Response::Value::get(std::string &value) const {
		value = this->value;
		return *this;
	}

 }
 */