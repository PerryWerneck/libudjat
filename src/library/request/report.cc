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

/*
 #include <config.h>
 #include <private/request.h>
 #include <udjat/tools/abstract/response.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/response/table.h>
 #include <cstdarg>
 #include <iomanip>
 #include <sstream>

 namespace Udjat {

	Response::Table::Table(const MimeType mimetype) : Abstract::Response{mimetype} {
	}

	Response::Table::~Table() {
	}

	std::string Response::Table::to_string() const {
		std::stringstream out;
		debug(__FUNCTION__,": Serializing table");
		serialize(out);
		debug("Serialized");
		return out.str();
	}

	void Response::Table::serialize(std::ostream &stream) const {

		Abstract::Response::serialize(stream);

		switch(mimetype) {
		case MimeType::xml:

			stream << "</response>";

			if(!info.caption.empty()) {
				stream << "<caption>" << info.caption << "</caption>";
			}

			if(empty()) {

				stream << "<contents />";

			} else {

				stream << "<contents><item>";

				auto column = columns.names.begin();
				for_each([this, &stream, &column](const Value::Type, const char *value){

					if(column == columns.names.end()) {
						stream << "</item><item>";
						column = columns.names.begin();
					}
					stream << "<" << *column << ">" << value << "</" << *column << ">";
					column++;

				});

				stream << "</item></contents>";
			}

			stream << "</response>";
			break;

		case Udjat::MimeType::json:
			{
				stream << "[{";

				bool sep = false;
				auto column = columns.names.begin();
				for_each([this, &stream, &column,&sep](const Value::Type type, const char *value){

					if(column == columns.names.end()) {
						stream << "},{";
						column = columns.names.begin();
						sep = false;
					}

					if(sep) {
						stream << ',';
					}
					sep = true;

					stream << "\"" << column->c_str() << "\":";

					switch(type) {
					case Value::Undefined:
					case Value::Array:
					case Value::Object:
						stream << "false";
						break;

					case Value::Signed:
					case Value::Unsigned:
					case Value::Real:
					case Value::Fraction:
						stream << value;
						break;

					default:
						stream << "\"" << value << "\"";

					}

					stream << endl;
					column++;

				});

				stream << "}]}";

			}
			break;

		case Udjat::MimeType::html:
			{
				stream << "<table>";
				if(!info.caption.empty()) {
					stream << "<caption>" << info.caption << "</caption>";
				}
				stream << "<thead><tr>";

				for(const auto &column : columns.names) {
					stream << "<th>" << column << "</th>";
				}

				stream << "</tr></thead><tbody>";

				auto column = columns.names.begin();
				stream << "<tr>";
				for_each([this, &stream, &column](const Value::Type, const char *value){
					if(column == columns.names.end()) {
						stream << "</tr><tr>";
						column = columns.names.begin();
					}
					stream << "<td>";
					stream << value;
					stream << "</td>";
					column++;
				});
				stream << "</tr>";

				stream << "</tbody></table>";

				debug("Serialized");

			}
			break;

		default:
			throw runtime_error(Logger::String{"Unable to serialize table to ",std::to_string(mimetype)});
		}

	}

	void Response::Table::start(const char *column_name, ...) {
		va_list args;
		va_start(args, column_name);
		set(column_name,args);
		va_end(args);
	}

	void Response::Table::start(const std::vector<string> &column_names) {

		if(!this->columns.names.empty()) {
			throw system_error(EBUSY,system_category(),"Report already started");
		}

		this->columns.names = column_names;

		open();
	}

	void Response::Table::set(const char *column_name, va_list args) {

		if(!columns.names.empty()) {
			throw system_error(EBUSY,system_category(),"Report already started");
		}

		while(column_name) {
			columns.names.push_back(column_name);
			column_name = va_arg(args, const char *);
		}

		open();
	}

	bool Response::Table::open() {

		if(columns.current == columns.names.begin()) {
			return false;
		}

		columns.current = columns.names.begin();
		return true;
	}

	bool Response::Table::close() {
		if(columns.current == columns.names.begin()) {
			return false;
		}
		return true;
	}

	std::string Response::Table::next() {

		if(columns.current == columns.names.end()) {
			close();
			open();
		}

		return (columns.current++)->c_str();

	}

	Response::Table & Response::Table::push_back(const Udjat::Value &values) {

		for(const std::string &column : columns.names) {

			if(!values.for_each([this,&column](const char *name, const Udjat::Value &value){

				if(strcasecmp(name,column.c_str())) {
					return false;
				}

				// Found, insert it.
				this->push_back(value.to_string(),(Value::Type) value);
				return true;

			})) {

				// Not found, insert 'null' value.
				this->push_back("",Udjat::Value::Undefined);

			}

		}

		return *this;

	}

	Response::Table & Response::Table::push_back(const std::string &str, Udjat::Value::Type type) {
		push_back(str.c_str(),type);
		return *this;
	}

	Response::Table & Response::Table::push_back(const short value) {
		return push_back(std::to_string((int) value),Value::Type::Signed);
	}

	Response::Table & Response::Table::push_back(const unsigned short value) {
		return push_back(std::to_string(value),Value::Type::Unsigned);
	}

	Response::Table & Response::Table::push_back(const int value) {
		return push_back(std::to_string(value),Value::Type::Signed);
	}

	Response::Table & Response::Table::push_back(const unsigned int value) {
		return push_back(std::to_string(value),Value::Type::Unsigned);
	}

	Response::Table & Response::Table::push_back(const long value) {
		return push_back(std::to_string(value),Value::Type::Signed);
	}

	Response::Table & Response::Table::push_back(const unsigned long value) {
		return push_back(std::to_string(value),Value::Type::Unsigned);
	}

	Response::Table & Response::Table::push_back(const TimeStamp value) {
		return push_back(value.to_string(),Value::Type::Timestamp);
	}

	Response::Table & Response::Table::push_back(const bool value) {
		return push_back(value ? "1" : "0",Value::Type::Boolean);
	}

	Response::Table & Response::Table::push_back(const float value) {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << value;
		return push_back(stream.str(), Value::Type::Real);
	}

	Response::Table & Response::Table::push_back(const double value) {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << value;
		return push_back(stream.str(),Value::Type::Real);
	}

 }

*/

