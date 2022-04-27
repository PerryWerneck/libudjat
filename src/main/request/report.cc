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

	Report::Report() {
	}

	Report::~Report() {
	}

	void Report::start(const char *name, const char *column_name, ...) {
		va_list args;
		va_start(args, column_name);
		set(column_name,args);
		va_end(args);
	}

	void Report::start(const char *name, const std::vector<string> &column_names) {

		if(!this->columns.names.empty()) {
			throw system_error(EBUSY,system_category(),"Report already started");
		}

		this->columns.names = column_names;

		open();
	}

	void Report::set(const char *column_name, va_list args) {

		if(!columns.names.empty()) {
			throw system_error(EBUSY,system_category(),"Report already started");
		}

		while(column_name) {
			columns.names.push_back(column_name);
			column_name = va_arg(args, const char *);
		}

		open();
	}

	bool Report::open() {

		if(columns.current == columns.names.begin()) {
			return false;
		}

		columns.current = columns.names.begin();
		return true;
	}

	bool Report::close() {
		if(columns.current == columns.names.begin()) {
			return false;
		}
		return true;
	}

	std::string Report::next() {

		if(columns.current == columns.names.end()) {
			close();
			open();
		}

		return (columns.current++)->c_str();

	}

	Report & Report::push_back(const std::string &str) {
		push_back(str.c_str());
		return *this;
	}

	Report & Report::push_back(const short value) {
		return push_back(std::to_string((int) value));
	}

	Report & Report::push_back(const unsigned short value) {
		return push_back(std::to_string(value));
	}

	Report & Report::push_back(const int value) {
		return push_back(std::to_string(value));
	}

	Report & Report::push_back(const unsigned int value) {
		return push_back(std::to_string(value));
	}

	Report & Report::push_back(const long value) {
		return push_back(std::to_string(value));
	}

	Report & Report::push_back(const unsigned long value) {
		return push_back(std::to_string(value));
	}

	Report & Report::push_back(const TimeStamp value) {
		return push_back(value.to_string(TIMESTAMP_FORMAT_JSON));
	}

	Report & Report::push_back(const bool value) {
		return push_back(value ? 1 : 0);
	}

	Report & Report::push_back(const float value) {
		return push_back(std::to_string(value));
	}

	Report & Report::push_back(const double value) {
		return push_back(std::to_string(value));
	}

 }

