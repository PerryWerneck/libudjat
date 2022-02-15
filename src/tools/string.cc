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

 #include <udjat/tools/string.h>
 #include <udjat/tools/timestamp.h>
 #include <cstring>
 #include <ctype.h>
 #include <cstdlib>

 using namespace std;

 namespace Udjat {

	String & String::strip() noexcept {
		char *ptr = strdup(c_str());
		assign(Udjat::strip(ptr));
		free(ptr);
		return *this;
	}

	String & String::chug() noexcept {
		char *ptr = strdup(c_str());
		assign(Udjat::chug(ptr));
		free(ptr);
		return *this;
	}

	String & String::chomp() noexcept {
		char *ptr = strdup(c_str());
		assign(Udjat::chomp(ptr));
		free(ptr);
		return *this;
	}

	/*
	String & expand() {
		return expand([](std::string &str){return false;});
	}
	*/

	static std::string getarguments(const std::string &key, const char *def) {

		const char *from = strchr(key.c_str(),'(');
		if(!from) {
			return def;
		}

		const char *to = strchr(++from,')');
		if(!from) {
			throw runtime_error(string{"Invalid expression '"} + key + "'");
		}

		return string(from,to-from);
	}

	String & String::expand(const Udjat::Abstract::Object &object, bool dynamic, bool cleanup) {
		return expand([&object](const char *key, std::string &str){
			return object.getProperty(key,str);
		},dynamic,cleanup);
	}

	String & String::expand(const std::function<bool(const char *key, std::string &str)> &expander, bool dynamic, bool cleanup) {

		auto from = find("${");
		while(from != string::npos) {

			auto to = find("}",from+3);
			if(to == string::npos) {
				throw runtime_error("Invalid ${} usage");
			}

			string value;
			string key(c_str()+from+2,(to-from)-2);
			if(expander(key.c_str(),value)) {

				// Got value, apply it.
				replace(
					from,
					(to-from)+1,
					value.c_str()
				);

				from = find("${",from);

			} else if(dynamic && strncasecmp(key.c_str(),"timestamp",9) == 0) {

				replace(
					from,
					(to-from)+1,
					TimeStamp().to_string(getarguments(key,"%x %X")).c_str()
				);

				from = find("${",from);

			} else {

				const char *env = getenv(key.c_str());

				if(env) {

					replace(
						from,
						(to-from)+1,
						env
					);

					from = find("${",from);

				} else if(cleanup) {

					replace(
						from,
						(to-from)+1,
						""
					);

					from = find("${",from);

				} else {
					// No value, skip.
					from = find("${",to+1);
				}


			}

		}

		return *this;
	}

	std::vector<String> String::split(const char *delim) {

		std::vector<String> strings;

		const char *ptr = c_str();
		while(ptr && *ptr) {
			const char *next = strstr(ptr,delim);
			if(!next) {
				strings.push_back(String(ptr).strip());
				break;
			}

			while(*next && isspace(*next))
				next++;

			strings.push_back(String(ptr,(size_t) (next-ptr)).strip());
			ptr = next+1;
			while(*ptr && isspace(*ptr)) {
				ptr++;
			}

		}

		return strings;

	}

 	char * chomp(char *str) noexcept {

		size_t len = strlen(str);

		while(len--) {

			if(isspace(str[len])) {
				str[len] = 0;
			} else {
				break;
			}
		}

		return str;

	}

	char * chug (char *str) noexcept {

		char *start;

		for (start = (char*) str; *start && isspace(*start); start++);

		memmove(str, start, strlen ((char *) start) + 1);

		return str;
	}

	char * strip(char *str) noexcept {
		return chomp(chug(str));
	}

	std::string & strip(std::string &str) noexcept {
		char *buffer = new char[str.size()+1];
		memcpy(buffer,str.c_str(),str.size());
		buffer[str.size()] = 0;
		strip(buffer);
		str.assign(buffer);
		delete[] buffer;
		return str;
	}

	std::string UDJAT_API strip(const char *str, ssize_t length) {

		if(length < 0) {
			length = strlen(str);
		}

		char *buffer = new char[length+1];
		memcpy(buffer,str,length);
		buffer[length] = 0;
		strip(buffer);

		std::string rc(buffer);
		delete[] buffer;

		return rc;
	}


 }

