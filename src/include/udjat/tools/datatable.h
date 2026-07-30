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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/http/status.h>
 #include <string>
 #include <ostream>
 #include <vector>
 #include <list>
 #include <cstdarg>
 #include <functional>

 namespace Udjat {

	/// @brief Abstract object containing values ordered in rows & columns.
	class UDJAT_API DataTable : public HTTP::Status {

	protected:
		size_t row = 0;
		size_t col = 0;

		std::vector<std::string> columns;
		std::string table_caption;

		virtual DataTable & next() noexcept;

	public:
		DataTable() = default;

		DataTable & open(std::vector<std::string> &column_names);
		DataTable & open(const char *column_name, ...) __attribute__ ((sentinel));

		virtual ~DataTable();

		inline void caption(const char *str) noexcept {
			table_caption.assign(str);
		}

		inline const char *caption() const noexcept {
			return table_caption.c_str();
		}

		virtual DataTable & push_back(size_t row, size_t col, const char *column_name, const Value &value) = 0;

		virtual DataTable & push_back(const Value &value);

		virtual DataTable & push_back(const char * value);
		virtual DataTable & push_back(const short value);
		virtual DataTable & push_back(const unsigned short value);
		virtual DataTable & push_back(const int value);
		virtual DataTable & push_back(const unsigned int value);
		virtual DataTable & push_back(const TimeStamp &value);
		virtual DataTable & push_back(const bool value);
		virtual DataTable & push_back(const float value);
		virtual DataTable & push_back(const double value);

	};

 }

 namespace std {

	template <typename T>
	inline Udjat::DataTable & operator<<(Udjat::DataTable &table, T value) {
		return table.push_back(value);
	}

 }
