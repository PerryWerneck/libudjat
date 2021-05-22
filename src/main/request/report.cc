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

 #include "private.h"
 #include <cstdarg>

 namespace Udjat {

	Response::Report::Report() {
	}

	Response::Report::~Report() {
	}

	void Response::Report::start(const char *name, const char *column_name, ...) {
		va_list args;
		va_start(args, column_name);
		set(column_name,args);
		va_end(args);
	}

	void Response::Report::set(const char *column_name, va_list args) {

		while(column_name) {
			columns.names.push_back(column_name);
			column_name = va_arg(args, const char *);
		}

		open();
	}

	bool Response::Report::open() {

		if(columns.current == columns.names.begin()) {
			return false;
		}

		columns.current = columns.names.begin();
		return true;
	}

	bool Response::Report::close() {
		if(columns.current == columns.names.begin()) {
			return false;
		}
		return true;
	}

	std::string Response::Report::next() {

		if(columns.current == columns.names.end()) {
			close();
			open();
		}

		return (columns.current++)->c_str();

	}

	Response::Report & Response::Report::push_back(const std::string &str) {
		push_back(str.c_str());
		return *this;
	}

	Response::Report & Response::Report::push_back(const bool value) {
		return push_back(value ? 1 : 0);
	}

	Response::Report & Response::Report::push_back(const TimeStamp &timestamp) {
		return push_back(timestamp.to_string(TIMESTAMP_FORMAT_JSON));
	}

	Response::Report & Response::Report::push_back(const int8_t value) {
		return push_back(std::to_string(value));
	}

	Response::Report & Response::Report::push_back(const int16_t value) {
		return push_back(std::to_string(value));
	}

	Response::Report & Response::Report::push_back(const int32_t value) {
		return push_back(std::to_string(value));
	}

	Response::Report & Response::Report::push_back(const uint8_t value) {
		return push_back(std::to_string(value));
	}

	Response::Report & Response::Report::push_back(const uint16_t value) {
		return push_back(std::to_string(value));
	}

	Response::Report & Response::Report::push_back(const uint32_t value) {
		return push_back(std::to_string(value));
	}

 }

