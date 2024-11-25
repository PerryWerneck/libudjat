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
 #include <udjat/tools/report.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/intl.h>
 #include <cstdarg>
 #include <stdexcept>
 #include <iomanip>

 using namespace std;

 namespace Udjat {

	Report::Report(const char *column_name, ...) {
		va_list args;
		va_start(args, column_name);
		set_headers(column_name,args);
		va_end(args);		
	}

	Report::Report(const Udjat::Value &first_row) {
		first_row.for_each([&](const char *name, const Value &value){
			headers.emplace_back(name);
			auto cell = cells.emplace_back();
			cell.set(value);
			return false;
		});

	}

	Report::~Report() {
	}

	void Report::set_headers(const char *column_name, va_list args) {
		if(!headers.empty()) {
			throw system_error(EBUSY,system_category(),"Report already started");
		}
		while(column_name) {
			headers.emplace_back(column_name);
			column_name = va_arg(args, const char *);
		}
	}

	Report::Cell::Cell() {
	}

	Report::Cell::~Cell() {
		clear();
	}

	void Report::Cell::clear() noexcept {
		if(data.ptr) {
			if(type == Value::String || type == Value::Url || type == Value::Icon) {
				free(data.ptr);
			}
			data.ptr = nullptr;
		}
	}

	void Report::Cell::set(const Value &value) {
		clear();
		type = (Value::Type) value;
		switch(type) {
		case Value::Undefined:
			break;

		case Value::String:
		case Value::Icon:
		case Value::Url:
			data.ptr = strdup(value.c_str());
			break;

		case Value::Timestamp:
			value.get(data.timestamp);
			break;

		case Value::State:
		case Value::Signed:
			value.get(data.sig);
			break;

		case Value::Unsigned:
		case Value::Boolean:
			value.get(data.unsig);
			break;

		case Value::Real:
		case Value::Fraction:
			value.get(data.dbl);
			break;

		default:
			throw logic_error("Invalid value type for report cell");

		}

	}


	void Report::Cell::set(const char *value) {
		clear();
		type = Value::String;
		data.ptr = strdup(value);
	}

	void Report::Cell::set(const short value) {
		clear();
		type = Value::Signed;
		data.sig = value;
	}

	void Report::Cell::set(const unsigned short value) {
		clear();
		type = Value::Unsigned;
		data.unsig = value;
	}

	void Report::Cell::set(const int value) {
		clear();
		type = Value::Signed;
		data.sig = value;		
	}

	void Report::Cell::set(const unsigned int value) {
		clear();
		type = Value::Unsigned;
		data.unsig = value;
	}

	void Report::Cell::set(const TimeStamp &value) {
		clear();
		type = Value::Timestamp;
		data.timestamp = (time_t) value;		
	}

	void Report::Cell::set(const bool value) {
		clear();
		type = Value::Boolean;
		data.unsig = value;
	}

	void Report::Cell::set(const float value) {
		clear();
		type = Value::Real;
		data.dbl = value;
	}

	void Report::Cell::set(const double value) {
		clear();
		type = Value::Real;
		data.dbl = value;
	}

	void Report::Cell::serialize(std::ostream &out) const {

		switch(type) {
		case Value::Undefined:
			break;

		case Value::Array:
		case Value::Object:
		case Value::Report:
			throw logic_error("Unable to convert value");
			break;

		case Value::Icon:
		case Value::Url:
		case Value::String:
			out << ((const char *) data.ptr);
			break;

		case Value::Timestamp:
			out << TimeStamp{data.timestamp};
			break;

		case Value::Signed:
			out << data.sig;
			break;

		case Value::Boolean:
			out << (data.sig ? _("Yes") : _("No"));
			break;

		case Value::Unsigned:
		case Value::State:
			out << data.unsig;
			break;

		case Value::Real:
			out << std::fixed << std::setprecision(2) << data.dbl;
			break;

		case Value::Fraction:
			out << std::fixed << std::setprecision(2) << (data.dbl * 100);
			break;

		default:
			throw logic_error("The column type is unexpected or invalid");
		}

	}

	void Report::to_json(std::ostream &out) const {

		out << "[";

		if(!cells.empty()) {

			out << "{";
			bool sep = false;
			auto column = headers.begin();
			for(const auto &cell : cells ) {

				if(column == headers.end()) {
					out << "},{";
					sep = false;
					column = headers.begin();
				}

				if(sep) {
					out << ',';
				}
				sep = true;

				out << "\"" << *column << "\":";

				if(cell.type == Value::String || cell.type == Value::Url || cell.type == Value::Icon || cell.type == Value::Timestamp) {
					out << "\"";
					cell.serialize(out);
					out << "\"";
				} else if(cell.type == Value::Undefined) {
					out << "false";
				} else if(cell.type == Value::Boolean) {
					out << (cell.data.sig ? "true" : "false");
				} else {
					cell.serialize(out);
				}

				column++;

			}

			out << "}";
		}

		out << "]";

	}

	void Report::to_xml(std::ostream &out) const {

		if(!field.caption.empty()) {
			out << "<caption>" << field.caption << "</caption>";
		}

		if(!cells.empty()) {

			auto column = headers.begin();

			out << "<item>";
			for(const auto &cell : cells ) {

				if(column == headers.end()) {
					out << "</item><item>";
					column = headers.begin();
				}

				out << "<" << *column << ">";
				cell.serialize(out);
				out << "</" << *column << ">";

				column++;
			}

			out << "</item>";

		}
	}

	void Report::to_html(std::ostream &out) const {

		out << "<table>";
		if(!field.caption.empty()) {
			out << "<caption>" << field.caption << "</caption>";
		}
		out << "<thead><tr>";

		for(const auto &header : headers) {
			out << "<th>" << header << "</th>";
		}

		out << "</tr></thead>";
		
		if(cells.empty()) {
			out << "<tbody class=\"no-data\" />";
		} else {
			out << "<tbody>";

			auto column = headers.begin();

			out << "<tr>";
			for(const auto &cell : cells ) {

				if(column == headers.end()) {
					out <<  "</tr><tr>";
					column = headers.begin();
				}

				if(cell.type == Value::Boolean) {
					out << "<td class=\"" << (cell.data.sig ? "true-value" : "false-value") << "\">";
				} else {
					out << "<td class=\"" << std::to_string(cell.type) << "\">";
				}

				cell.serialize(out);
				out << "</td>";

				column++;
			}

			out << "</tr></tbody>";
		}

		out << "</table>";
	}

	void Report::to_yaml(std::ostream &out, size_t left_margin) const {

	}

	void Report::to_sh(std::ostream &stream) const {

	}

	void Report::to_csv(std::ostream &out, char delimiter) const {

	}


 }