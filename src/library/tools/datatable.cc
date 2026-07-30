/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/datatable.h>
 #include <udjat/tools/intl.h>
 #include <cstdarg>
 #include <stdexcept>
 #include <iomanip>
 #include <system_error>

 using namespace std;

 namespace Udjat {

	DataTable & DataTable::open(std::vector<std::string> &column_names) {
		columns = column_names;
		return *this;
	}

	DataTable & DataTable::open(const char *column_name, ...) {
		va_list args;
		va_start(args, column_name);
		while(column_name) {
			columns.emplace_back(column_name);
			column_name = va_arg(args, const char *);
		}
		va_end(args);	
		return *this;	
	}

	DataTable::~DataTable() {
	}

	DataTable & DataTable::next() noexcept {
		if(++col > columns.size()) {
			col = 0;
			row++;
		}
		return *this;
	}

	DataTable & DataTable::push_back(const Value &value) {
		push_back(row,col,columns[col].c_str(),value);
		return next();
	}

	DataTable & DataTable::push_back(const char * value) {
		Value v;
		v = value;
		return push_back(v);
	}

	DataTable & DataTable::push_back(const short value) {
		Value v;
		v = value;
		return push_back(v);
	}

	DataTable & DataTable::push_back(const unsigned short value) {
		Value v;
		v = value;
		return push_back(v);
	}

	DataTable & DataTable::push_back(const int value) {
		Value v;
		v = value;
		return push_back(v);
	}

	DataTable & DataTable::push_back(const unsigned int value) {
		Value v;
		v = value;
		return push_back(v);
	}

	DataTable & DataTable::push_back(const TimeStamp &value) {
		Value v;
		v = value;
		return push_back(v);
	}

	DataTable & DataTable::push_back(const bool value) {
		Value v;
		v = value;
		return push_back(v);
	}

	DataTable & DataTable::push_back(const float value) {
		Value v;
		v = value;
		return push_back(v);
	}

	DataTable & DataTable::push_back(const double value) {
		Value v;
		v = value;
		return push_back(v);
	}

 }
