/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <private/variant.h>
 #include <udjat/tools/variant.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Variant & Variant::add_column(const char *name, const Type type) {

		if(this->type != DataTable) {
			reset(DataTable);
		}

		((Variant::Table *) content.ptr)->add_column(name,type);

		return *this;
	}


	Variant::Table::Table() {
		rewind();
	}

	Variant::Table::~Table() {
		clear();
	}

	void Variant::Table::clear() {
		rewind();
		for(auto &item : itens) {
			Variant::clear(it->type,item);
			next();
		}
		itens.clear();
		rewind();
	}

	void Variant::Table::next() {
		it++;
		if(it == columns.end()) {
			it = columns.begin();
		}
	}

	Variant::Content & Variant::Table::append_type(const Variant::Type type) {

		if(itens.empty()) {
			rewind();
		}

		if(type != it->type) {
			throw logic_error("Unexpected type append entry to table");
		}

		debug("Inserting column '",it->name.c_str(),"'");

		itens.emplace_back();
		next();
		return itens.back();
		
	}

	void Variant::Table::for_each(const std::function<void(const char *name, const Variant::Type type, const Variant::Content &value)> &callback) {

		auto column = columns.begin();
		for(const auto &item : itens) {
			if(column == columns.end()) {
				column = columns.begin();
			}
			callback(column->name.c_str(),column->type,item);
			column++;
		}

	}

 }


