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
 #include <udjat/tools/value.h>
 #include <udjat/tools/logger.h>
 #include <iostream>
 #include <vector>

 using namespace std;

 namespace Udjat {

	void Value::to_html(std::ostream &ss) const {

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wswitch"
		switch((Value::Type) *this) {
		case Udjat::Value::Object:

			if(!empty()) {

				ss << "<ul>";
				for_each([&ss](const char *name, const Value &value){

					ss << "<li><label>" << name << ":&nbsp;";
					value.to_html(ss);
					ss << "</label></li>";
					return false;

				});
				ss << "</ul>";
			}
			break;

		case Udjat::Value::Array:

			if(!empty()) {

				vector<string> colnames;
				ss << "<table><thead><tr>";
				for_each([&ss,&colnames](const char *, const Value &row){
					row.for_each([&ss,&colnames](const char *name, const Value &){
						colnames.push_back(name);
						ss << "<th>" << name << "</th>";
						return false;
					});
					return true;
				});
				ss << "</tr></thead><tbody>";

				for_each([&ss,&colnames](const char *, const Udjat::Value &row){
					ss << "<tr>";

					for(const string &name : colnames) {
						ss << "<td>";
						row[name.c_str()].to_html(ss);
						ss << "</td>";
					}
					ss << "</tr>";
					return false;
				});

				ss << "</tbody></table>";
			}
			break;

		case Udjat::Value::Signed:
		case Udjat::Value::Unsigned:
		case Udjat::Value::Real:
		case Udjat::Value::Fraction:
			ss << "<strong class='numeric-value'>" << to_string() << "</strong>";
			break;

		case Udjat::Value::Boolean:
			ss << "<strong class='" << (as_bool() ? "true-value" : "false-value") << "'>" << to_string() << "</strong>";
			break;

		case Udjat::Value::Url:
			ss << "<strong class='string-value'>";
			if(!empty()) {
				ss << "<a href='" << to_string() << "'>" << to_string() << "</a>";
			}
			ss << "</strong>";
			break;

		default:
			ss << "<strong class='string-value'>" << to_string() << "</strong>";

		}
		#pragma GCC diagnostic pop

	}

	/*
 #include <udjat/civetweb.h>
 #include <udjat/tools/http/value.h>
 #include <iostream>
 #include <iomanip>
 #include <vector>

 using namespace std;

 namespace Udjat {

	void HTTP::Value::html(std::stringstream &ss) const {

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wswitch"
		switch(this->type) {
		case Udjat::Value::Object:
			{
				ss << "<ul>";
				for(auto &child : children) {
					ss << "<li><label>" << child.first << ":&nbsp;";
					if(child.second->type == Udjat::Value::Object || child.second->type == Udjat::Value::Array ) {
						child.second->html(ss);
					} else if(child.second->type == Udjat::Value::Fraction ) {
						ss << "<strong>" << child.second->to_string() << "%</strong>";
					} else {
						ss << "<strong>" << child.second->to_string() << "</strong>";
					}

					ss << "</label></li>";
				}
				ss << "</ul>";
			}
			break;

		case Udjat::Value::Array:

			if(!children.empty()) {

				ss << "<table>";

				// Get column names.
				vector<string> colnames;
				{
					const Value &first = *children.begin()->second;
					if(!first.children.empty()) {
						ss << "<thead><tr>";
						for(auto &col : first.children) {
							ss << "<th>";
							colnames.push_back(col.first);
							ss << col.first;
							ss << "</th>";
						}
						ss << "</tr></thead>";
					}
				}

				// Get column contents.
				ss << "<tbody>";
				{
					for(auto &row : children) {
						if(!row.second->children.empty()) {
							ss << "<tr>";
							for(string &colname : colnames) {
								auto value = row.second->children.find(colname.c_str());
								ss << "<td>";
								if(value != row.second->children.end()) {
									ss << value->second->to_string();
								}
								ss << "</td>";
							}
							ss << "</tr>";
						}
					}
				}
				ss << "</tbody></table>";
			}
			break;

		}
		#pragma GCC diagnostic pop

	}
 */

 }

