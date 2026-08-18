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

#pragma once

#include <config.h>
#include <udjat/defs.h>
#include <udjat/tools/variant.h>
#include <vector>
#include <string>

namespace Udjat {

	class UDJAT_PRIVATE Variant::Table {
	private:

		struct Info {
			std::string name;
			Variant::Type type;

			Info(const char *n, const Variant::Type t) : name{n}, type{t} {				
			}

		};

		/// @brief The table columns.
		std::vector<Info> columns;

		/// @brief Iterator
		std::vector<Info>::const_iterator it;

		/// @brief Table columns.
		std::vector<Variant::Content> itens;

	public:
		Table();
		~Table();

		inline void rewind() {
			it = columns.begin();
		}

		void clear();

		void next();

		Variant::Content & append_type(const Variant::Type type);

		inline void add_column(const char *name, const Variant::Type type) {
			columns.emplace_back(name,type);
		} 

		void for_each(const std::function<void(size_t column, const char *name, const Variant::Type type, const Variant::Content &value)> &callback);
		void for_each(const std::function<void(const char *name, const Variant::Type type)> &callback) const;

	};

}
