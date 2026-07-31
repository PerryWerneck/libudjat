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
  * @brief Brief Convert value to name=value list.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/mimetype.h>
 #include <iostream>
 #include <vector>

 using namespace std;

 namespace Udjat {

	void Value::to_sh(std::ostream &stream) const {

		if(empty()) {
			return;
		}

		if(*this != Udjat::Value::Object) {
			throw runtime_error(Logger::String{"Only objects can be serialized as ",std::to_string(MimeType::csv)});
		}

		// Get headers.
		for_each([&stream](const char *key, const Udjat::Value &value){

			switch((Value::Type) value) {
			case Udjat::Value::Undefined:
			case Udjat::Value::Array:
			case Udjat::Value::Object:
				break;

			case Udjat::Value::Signed:
			case Udjat::Value::Unsigned:
			case Udjat::Value::Real:
			case Udjat::Value::Boolean:
			case Udjat::Value::Fraction:
				stream << key << "=" << value << endl;
				break;

			default:
				stream << key << "=\"" << value << "\"" << endl;
			}
			return false;

		});

	}
 }
