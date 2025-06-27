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
 #include <udjat/tools/value.h>
 #include <udjat/tools/report.h>
 #include <udjat/tools/logger.h>
 #include <iostream>

 namespace Udjat {

	void Value::to_xml(std::ostream &ss) const {

		switch((Value::Type) *this) {
		case Udjat::Value::Undefined:
			break;

		case Udjat::Value::Array:
			for_each([&ss](const char *key, const Value &value){
				ss << "<item name='" << key << "' type='" << std::to_string((Udjat::Value::Type) value) << "'"<< ">";
				value.to_xml(ss);
				ss << "</item>";
				return false;
			});
			break;

		case Udjat::Value::Object:
			for_each([&ss](const char *key, const Value &value){
				ss << "<" << key << " type='"; 
				if(value != Report) {
					ss << std::to_string((Udjat::Value::Type) value);
				} else {
					ss << std::to_string(Udjat::Value::Array);
				}
				ss << "'"<< ">";
				value.to_xml(ss);
				ss << "</" << key << ">";
				return false;
			});
			break;

		case Udjat::Value::Report:
			if(content.ptr) {
				((const Udjat::Report *) content.ptr)->to_xml(ss);
			} else {
				ss << "<contents />";
			}
			break;

		default:
			ss << to_string();
		}

	}

 }
