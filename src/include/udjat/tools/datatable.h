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
 #include <udjat/tools/schema.h>
 #include <udjat/tools/http/status.h>

 namespace Udjat {

	/// @brief Abstract object containing values ordered in rows & columns.
	class UDJAT_API DataTable : public HTTP::Status {
	protected:

		typedef DataTable super;

		const OutputSchema &schema;

		DataTable & next() noexcept;

		virtual void push_back(const Schema::Item &schema, const Value &value) = 0;

	public:
		DataTable(const OutputSchema &s);
		virtual ~DataTable();

		/// @brief Add caption for table.
		/// @param text The caption.
		/// @return true if the table can handle captions.
		virtual bool caption(const char *text);

		/// @brief Add foot for table.
		/// @param text The foot message.
		/// @return true if the table can handle foot.
		virtual bool foot(const char *text);
	
		/// @brief Add object with columns.
		/// @param value Object with column data to extract based on schema.
		virtual DataTable & add(const Value &value);

	};

 }

 namespace std {

	template <typename T>
	inline Udjat::DataTable & operator<<(Udjat::DataTable &table, T value) {
		return table.push_back(value);
	}

 }
