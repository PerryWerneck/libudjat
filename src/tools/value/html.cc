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
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/ui/icon.h>
 #include <iostream>
 #include <vector>
 #include <udjat/tools/file.h>

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
			try {
				Udjat::String tmpl{Config::Value<string>{"html","numeric-template","<strong class='numeric-value'>${value}</strong>"}.c_str()};

				ss << tmpl.expand([this](const char *key, std::string &value){

					if(!strcasecmp(key,"value")) {
						value = to_string();
						return true;
					}

					return false;

				},true,false);

			} catch(const std::exception &e) {
				Logger::String(to_string(),": ",e.what()).error("numeric");
			}
			break;

		case Udjat::Value::Boolean:
			try {
				Udjat::String tmpl{Config::Value<string>{"html","boolean-template","<strong class='${type}-value'>${value}</strong>"}.c_str()};

				ss << tmpl.expand([this](const char *key, std::string &value){

					if(!strcasecmp(key,"value")) {
						value = to_string();
						return true;
					} else if(!strcasecmp(key,"type")) {
						value = (as_bool() ? "true" : "false");
						return true;
					}

					return false;

				},true,false);

			} catch(const std::exception &e) {
				Logger::String(to_string(),": ",e.what()).error("icon");
			}
			break;

		case Udjat::Value::Url:
			try {
				if(!empty()) {

					Udjat::String tmpl{Config::Value<string>{"html","link-template","<strong class='string-value'><a href='${link}'>${link}</a>"}.c_str()};

					ss << tmpl.expand([this](const char *key, std::string &value){

						if(!strcasecmp(key,"link")) {
							value = to_string();
							return true;
						}

						return false;

					},true,false);

				} else {

					ss << "&nbsp;";

				}
			} catch(const std::exception &e) {
				Logger::String(to_string(),": ",e.what()).error("link");
			}
			break;

		case Udjat::Value::Icon:
			try {
				Udjat::String tmpl{Config::Value<string>{"html","icon-template","<strong class='icon-name'>${icon-name}</strong>"}.c_str()};

				tmpl.expand([this](const char *key, std::string &value){

					if(!strcasecmp(key,"icon-name")) {
						value = to_string();
						return true;
					}

					return false;

				},true,false);

				if(!strcasecmp(tmpl.c_str(),"embed")) {

					// Embed icon.
					File::Text path{Udjat::Icon{to_string().c_str()}.filename()};

					if(path.regular()) {
						ss << path.c_str();
					}

				} else {

					ss << tmpl;

				}

			} catch(const std::exception &e) {
				Logger::String(to_string(),": ",e.what()).error("icon");
			}
			break;

		default:
			try {
				Udjat::String tmpl{Config::Value<string>{"html","string-template","<strong class='string-value'>${value}</strong>"}.c_str()};

				ss << tmpl.expand([this](const char *key, std::string &value){

					if(!strcasecmp(key,"value")) {
						value = to_string();
						return true;
					}

					return false;

				},true,false);

			} catch(const std::exception &e) {
				Logger::String(to_string(),": ",e.what()).error("icon");
			}


		}
		#pragma GCC diagnostic pop

	}

 }

