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

 #include <iostream>
 #include <udjat/tools/string.h>
 #include <udjat/tools/system.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

	System::Config::File::File() {
		this->separator = '=';
	}

	System::Config::File::File(const char *filename, const char *separator) {

	 	if(separator[0]) {
			this->separator = separator[0];
	 	} else if(!strncasecmp(filename,"/proc/",6)) {
			this->separator = ':';
	 	} else {
			this->separator = '=';
	 	}

		set(Udjat::File::Text(filename).c_str());
	}

	std::string System::Config::File::name() const {

		char * filename = strdup(path.c_str());

		char * ptr = strrchr(filename,'.');
		if(ptr) {
			*ptr = 0;
		}

		char *name = strrchr(filename,'/');
		if(name) {
			ptr = strrchr(name+1,'\\');
			if(ptr) {
				name = (ptr+1);
			}
		}

		string response(strip(name));

		free(filename);

		if(response.empty()) {
			return "unnamed";
		}

		return response;
	}

	System::Config::File::Value System::Config::File::find(const char *key) const noexcept {

		for(auto value : values) {
			if(!strcasecmp(value.name.c_str(),key))
				return value;
		}

		return Value();
	}

	void System::Config::File::forEach(std::function<void(const System::Config::File::Value &value)> callback) const {
		for(auto value : values) {
			callback(value);
		}
	}

	static const char * find_start(const char *ptr) {
		while(*ptr && ::isspace(*ptr)) {
			ptr++;
		}
		return ptr;
	}

	System::Config::File & System::Config::File::set(const char *contents) {

		Value value;

		values.clear();

		Udjat::File::Text::for_each(contents, [this,&value](const string &line) {

			if(line[0] == '#') {

				if(line[1] != '#') {
					return;
				}

				const char *ptr = find_start(line.c_str()+2);

				if(!*ptr)
					return;

				if(!strncasecmp(ptr,"Description:",12)) {
					this->description = find_start(ptr+12);

				} else if(!strncasecmp(ptr,"Path:",5)) {
					this->path = find_start(ptr+5);

				} else if(!strncasecmp(ptr,"Type:",5)) {
					value.setType(find_start(ptr+5));

				}

			} else {

				const char *ptr = find_start(line.c_str());
				if(!*ptr)
					return;

				const char * separator = strchr(ptr,this->separator);

				if(!separator)
					return;

				value.name = Udjat::strip(ptr,separator-ptr);

				separator = find_start(++separator);
				if(*separator=='\"' || *separator=='\'') {

					const char *last = strrchr((separator+1),*separator);
					if(!last) {
						cerr << "\tMalformed sysconfig entry '" << line << "'" << endl;
					} else {
						separator++;
						value.value = string(separator,last-separator);
					}

				} else {
					value.value = separator;
				}

				values.push_back(value);

			}

		});

		return *this;

	}

 }

