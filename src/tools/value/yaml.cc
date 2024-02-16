/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <iostream>

 using namespace std;

 namespace Udjat {

	void Value::to_yaml(std::ostream &ss, size_t left_margin) const {

		switch((Value::Type) *this) {
		case Udjat::Value::Undefined:
			break;

		case Udjat::Value::Array:
			if(left_margin) {
				ss << endl;
			}
			for_each([&ss,left_margin](const char *, const Value &value){
				std::string spaces;
				spaces.resize(left_margin,' ');
				ss << spaces << "-";
				value.to_yaml(ss,left_margin+2);
				return false;
			});
			break;

		case Udjat::Value::Object:
			if(left_margin) {
				ss << endl;
			}
			for_each([&ss,left_margin](const char *key, const Value &value){
				std::string spaces;
				spaces.resize(left_margin,' ');
				ss << spaces << key << ":";
				value.to_yaml(ss,left_margin+4);
				return false;
			});
			break;

		case Udjat::Value::Signed:
		case Udjat::Value::Unsigned:
		case Udjat::Value::Real:
		case Udjat::Value::Boolean:
		case Udjat::Value::Fraction:
			ss << " " << to_string() << endl;
			break;

		default:
			ss << " \"" << to_string() << "\"" << endl;

		}

	}

	/*
	void HTTP::Value::yaml(std::stringstream &ss, size_t left_margin) const {

		switch(this->type) {
		case Udjat::Value::Undefined:
			break;

		case Udjat::Value::Array:
			if(left_margin) {
				ss << endl;
			}
			for(const auto [key, value] : children)	{
				std::string spaces;
				spaces.resize(left_margin,' ');
				ss << spaces << "-";
				value->yaml(ss,left_margin+2);
			}
			break;

		case Udjat::Value::Object:
			if(left_margin) {
				ss << endl;
			}
			for(const auto [key, value] : children)	{
				std::string spaces;
				spaces.resize(left_margin,' ');
				ss << spaces << key << ":";
				value->yaml(ss,left_margin+4);
			}
			break;

		case Udjat::Value::Signed:
		case Udjat::Value::Unsigned:
		case Udjat::Value::Real:
		case Udjat::Value::Boolean:
		case Udjat::Value::Fraction:
			ss << " " << this->value << endl;
			break;

		default:
			ss << " \"" << this->value << "\"" << endl;

		}

	}
	*/

 }


