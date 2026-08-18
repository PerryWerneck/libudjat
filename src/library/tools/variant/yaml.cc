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

 /**
  * @brief Brief Convert value to YAML string.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/variant.h>
 #include <udjat/tools/timestamp.h>
 #include <private/variant.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	void Variant::to_yaml(std::ostream &ss, size_t left_margin) const {

		switch((Variant::Type) *this) {
		case Udjat::Variant::Undefined:
			ss << " null" << endl;
			break;

		case Udjat::Variant::Array:
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

		case Udjat::Variant::DataTable:
			if(left_margin) {
				ss << endl;
			}

			(((Variant::Table *) content.ptr))->for_each([&ss,&left_margin](size_t column, const char *name, const Variant::Type type, const Variant::Content &content){
				std::string spaces;
				spaces.resize(left_margin,' ');
				ss << spaces;
				if(column == 0) {
					ss << "- ";
				} else {
					ss << "  ";
				}
				ss << name << ": " << Variant::to_string(type,content,MimeType::yaml) << endl;
			});
			break;

		case Udjat::Variant::ValueMap:
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

		case Udjat::Variant::Signed:
		case Udjat::Variant::Unsigned:
		case Udjat::Variant::Real:
		case Udjat::Variant::Boolean:
		case Udjat::Variant::Fraction:
			ss << " " << to_string() << endl;
			break;

		case Udjat::Variant::Timestamp:
			ss << " " << TimeStamp{content.timestamp}.to_string("%Y-%m-%dT%H:%M:%S%z") << endl;
			break;

		default:
			ss << " \"" << to_string() << "\"" << endl;

		}

	}

 }


