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
 #include <udjat/tools/value.h>
 #include <udjat/tools/report.h>
 #include <map>
 #include <vector>
 #include <string.h>

 using namespace std;

 namespace Udjat {
 
	Udjat::Report & Value::ReportFactory(const char *column_name, ... ) {

		clear();

		va_list args;
		va_start(args, column_name);
		Udjat::Report *worker = new Udjat::Report(column_name,args);
		va_end(args);		

		type = Report;
		content.ptr = (void *) worker;

		return *worker;

	}

	Udjat::Report & Value::ReportFactory(const std::vector<std::string> &column_names) {

		clear();

		Udjat::Report *worker = new Udjat::Report(column_names);
		type = Report;
		content.ptr = (void *) worker;

		return *worker;

	}

	Udjat::Report & Value::ReportFactory(const Value &first_row) {

		clear();

		Udjat::Report *worker = new Udjat::Report(first_row);
		type = Report;
		content.ptr = (void *) worker;

		return *worker;

	}

 }