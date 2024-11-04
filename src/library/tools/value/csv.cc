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
  * @brief Brief description of this source.
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

	void Value::to_csv(std::ostream &ss, char delimiter) const {

		if(*this != Udjat::Value::Array) {
			if(!for_each([&ss,delimiter](const char *, const Value &value) {
				if(value == Udjat::Value::Array) {
					value.to_csv(ss,delimiter);
					return true;
				}
				return false;
			})) {
				throw runtime_error(Logger::String{"Only arrays or object with an array can be serialized as ",std::to_string(MimeType::csv)});
			}

		}

		if(empty()) {
			return;
		}

		// Get headers.
		vector<string> colnames;
		bool first = true;
		for_each([&ss,&colnames,delimiter,&first](const char *, const Udjat::Value &row){

			if(first) {
				// First line, get column names.
				bool sep = false;
				row.for_each([&ss,&colnames,&sep,delimiter](const char *name, const Value &){
					if(sep) {
						ss << delimiter;
					}
					sep = true;
					colnames.push_back(name);
					ss << name;
					return false;
				});
				ss << endl;
				first = false;
			}

			// Print line
			{
				bool sep = false;
				for(const string &name : colnames) {
					if(sep) {
						ss << delimiter;
					}
					for(char chr : row[name.c_str()].to_string()) {
						if(chr != delimiter) {
							ss << chr;
						}
					}
				}
				ss << endl;
			}

			return false;

		});

	}
 }
