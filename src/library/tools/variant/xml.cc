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

 /**
  * @brief Brief Convert value to XML string.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/variant.h>
 #include <udjat/tools/logger.h>
 #include <private/variant.h>
 #include <iostream>

 namespace Udjat {

	void Variant::to_xml(std::ostream &ss) const {

		switch((Variant::Type) *this) {
		case Udjat::Variant::Undefined:
			break;

		case Udjat::Variant::Array:
			for_each([&ss](const char *key, const Value &value){
				ss << "<item name='" << key << "' type='" << std::to_string((Udjat::Variant::Type) value) << "'"<< ">";
				value.to_xml(ss);
				ss << "</item>";
				return false;
			});
			break;

		case Udjat::Variant::ValueMap:
			for_each([&ss](const char *key, const Value &value){
				ss << "<" << key << " type='"; 
				ss << std::to_string((Udjat::Variant::Type) value);
				ss << "'"<< ">";
				value.to_xml(ss);
				ss << "</" << key << ">";
				return false;
			});
			break;

		case Udjat::Variant::DataTable:
			{
				bool open = false;
				(((Variant::Table *) content.ptr))->for_each([&ss,&open](size_t column, const char *name, const Variant::Type type, const Variant::Content &content){
					if(column == 0) {
						ss << (open ? "</item><item>" : "<item>");
						open = true;
					}
					ss << "<" << name << " type='"; 
					ss << std::to_string((Udjat::Variant::Type) type);
					ss << "'"<< ">";
					ss << Variant::to_string(type,content,MimeType::json);
					ss << "</" << name << ">";
				});
				if(open) {
					ss << "</item>";
				}
			}
			break;
		
		default:
			ss << to_string();
		}

	}

 }
