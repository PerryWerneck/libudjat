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
 #include <udjat/tools/variant.h>
 #include <private/variant.h>
 #include <iostream>
 #include <cstring>

 using namespace std;

 namespace Udjat {

	void Variant::to_json(std::ostream &output) const {

		switch((Variant::Type) *this) {
		case Udjat::Variant::Undefined:
			output << "null";
			break;

		case Udjat::Variant::Array:
			{
				output << '[';
				bool sep = false;
				for_each([&output,&sep](const char *, const Value &value){
					if(sep) {
						output << ',';
					}
					sep = true;
					value.to_json(output);
					return false;
				});
				output << ']';
			}
			break;

		case Udjat::Variant::ValueMap:
			{
				output << '{';
				bool sep = false;
				for_each([&output,&sep](const char *name, const Value &value){
					if(sep) {
						output << ',';
					}
					sep = true;
					output << '"' << name << "\":";
					value.to_json(output);
					return false;
				});
				output << '}';
			}
			break;

		case Udjat::Variant::DataTable:
			{
				output << '[';
				bool open = false;
				(((Variant::Table *) content.ptr))->for_each([&output,&open](size_t column, const char *name, const Variant::Type type, const Variant::Content &content){
					if(column) {
						output << ",";
					} else {
						output << (open ? "},{" : "{");
						open = true;
					}
					output << '"' << name << "\":";
					if(Variant::isString(type)) {
						output << "\"" << Variant::to_string(type,content,MimeType::json) << "\"";
					} else {
						output << Variant::to_string(type,content,MimeType::json);
					}
				});
				if(open) {
					output << "}";
				}
				output << ']';
			}
			break;

		case Udjat::Variant::Signed:
		case Udjat::Variant::Unsigned:
		case Udjat::Variant::Real:
		case Udjat::Variant::Boolean:
			output << Variant::to_string(type,content,MimeType::json);
			break;

		case Udjat::Variant::Timestamp:
		case Udjat::Variant::Fraction:
			output << '"' << Variant::to_string(type,content,MimeType::json) << '"';

		default:
			// TODO: Convert special chars.
			output << '"' << Variant::to_string(type,content,MimeType::json) << '"';

		}

	}

 }


