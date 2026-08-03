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

	DataTable::DataTable(const Schema::Output &s) : schema{s} {
	}

	DataTable::~DataTable() {
	}

	bool DataTable::caption(const char *text) {
		return false;
	}

	bool DataTable::foot(const char *text) {
		return false;
	}

	std::ostream & DataTable::apply(std::ostream &stream, const MimeType mimetype, const std::function<void(DataTable &table)> &callback) {


		

		throw system_error(ENOTSUP,system_category(),"Unsupported MIME type");
	}

 }
