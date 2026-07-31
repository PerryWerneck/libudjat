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

		case Udjat::Variant::Object:
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

		case Udjat::Variant::Signed:
		case Udjat::Variant::Unsigned:
		case Udjat::Variant::Real:
		case Udjat::Variant::Boolean:
		case Udjat::Variant::Fraction:
			output << to_string();
			break;

		case Udjat::Variant::Timestamp:
			// Option 1: UTC Time with 'Z' Suffix (Recommended)This is the cleanest and most common JSON format. 
			// Force your time structure to UTC using std::gmtime, then hardcode the literal 'Z' at 
			// the end of the format string.
			{
				time_t now = content.timestamp;
				std::tm* gmt_time = std::gmtime(&now); // Convert to UTC
				char buffer[32];
				// Formats directly to: "2026-07-16T15:42:00Z"
				std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", gmt_time);
				output << '"' << buffer << '"';
			}

			// Option 2: Local Time with Manual Colon Insertion
			// If you must use local time, you have to use %z and manually insert the colon into 
			// the resulting string to make it compliant with standard JSON parsers.
			// {
			// 	string json_time = TimeStamp{content.timestamp}.to_string("%Y-%m-%dT%H:%M:%S%z");

			// 	// Manually fix the timezone format: -0400 -> -04:00
			// 	if (json_time.length() >= 5) {
			// 		json_time.insert(json_time.length() - 2, ":");
			// 	}

			// 	output << '"' << json_time << '"';	
			// }
			break;

		default:
			// TODO: Convert special chars.
			output << '"' << to_string() << '"';

		}

	}

 }

 /*
 #include <udjat/civetweb.h>
 #include <udjat/tools/http/value.h>
 #include <iostream>
 #include <iomanip>

 namespace Udjat {

	void HTTP::Variant::json(std::stringstream &ss) const {

		switch(this->type) {
		case Udjat::Variant::Undefined:
			ss << "null";
			break;

		case Udjat::Variant::Array:
			{
				ss << '[';

				bool sep = false;
				for(auto &child : children) {
					if(sep) {
						ss << ',';
					}
					sep = true;
					child.second->json(ss);
				}

				ss << ']';
			}
			break;

		case Udjat::Variant::Object:
			{
				ss << '{';

				bool sep = false;
				for(auto &child : children) {
					if(sep) {
						ss << ',';
					}
					sep = true;
					ss << '"' << child.first << "\":";
					child.second->json(ss);
				}

				ss << '}';
			}
			break;

		case Udjat::Variant::Signed:
		case Udjat::Variant::Unsigned:
		case Udjat::Variant::Real:
		case Udjat::Variant::Boolean:
		case Udjat::Variant::Fraction:
			ss << this->value;
			break;

		default:
			// TODO: Convert special chars.
			ss << '"' << this->value << '"';
		}

	}

 }
 */


