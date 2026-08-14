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
 #include <udjat/tools/variant.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/mimetype.h>
 #include <iostream>
 #include <vector>

 using namespace std;

 namespace Udjat {

	void Variant::to_sh(std::ostream &stream) const {

		if(empty()) {
			return;
		}

		if(*this != Udjat::Variant::ValueMap) {
			throw runtime_error(Logger::String{"Only value maps can be serialized as ",std::to_string(MimeType::csv)});
		}

		// Get headers.
		for_each([&stream](const char *key, const Udjat::Variant &value){

			switch((Variant::Type) value) {
			case Udjat::Variant::Undefined:
			case Udjat::Variant::Array:
			case Udjat::Variant::ValueMap:
				break;

			case Udjat::Variant::Signed:
			case Udjat::Variant::Unsigned:
			case Udjat::Variant::Real:
			case Udjat::Variant::Boolean:
			case Udjat::Variant::Fraction:
				stream << key << "=" << value << endl;
				break;

			default:
				stream << key << "=\"" << value << "\"" << endl;
			}
			return false;

		});

	}
 }
