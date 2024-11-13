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

 #include <udjat/tools/system.h>
 #include <udjat/tools/value.h>
 #include <iostream>
 #include <cstring>

 using namespace std;

 namespace Udjat {

	void System::Config::File::Value::clear() {
		type = String;
		def.clear();
		name.clear();
		value.clear();
	}

	void System::Config::File::Value::setType(const char *name) {
		static const struct {
			const char *name;
			Type type;
		} names[] = {
			{ "Boolean",	Boolean		},
			{ "YesNo", 		Boolean		},
			{ "Int",		Integer		},
			{ "Integer",	Integer		},
			{ "List", 		String		},
			{ "String", 	String		},
		};

		for(size_t ix = 0; ix < (sizeof(names)/sizeof(names[0]));ix++) {

			if(strncasecmp(names[ix].name,name,strlen(names[ix].name)))
				continue;

			type = names[ix].type;
			return;

		}

		clog << "Unexpected type '" << name << "' on sysconfig file, assuming 'string'" << endl;
		type = String;
	}

	Udjat::Value & System::Config::File::Value::get(Udjat::Value &value) const {

		switch(type) {
		case Value::Boolean:
			value = this->value;
			break;

		case Value::Integer:
			value = ::atoi(this->value.c_str());
			break;

		default:
			value = this->value;
		}

		return value;

	}


 }
