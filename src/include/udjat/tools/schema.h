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

 #include <udjat/defs.h>
 #include <udjat/tools/value.h>
 #include <vector>

 namespace Udjat {

	/// @brief Encapsulates the structural layout and validation rules used to parse, verify, and serialize structured data formats.
	class UDJAT_API Schema {
	public:

		class UDJAT_API Item {
		protected:
			const char *item_name;					///< @brief The item name.
			Value::Type item_type;					///< @brief The value type.
			const char *item_description;	///< @brief The item description.

		public:
			constexpr Item(const char *name, Value::Type type)
				: item_name{name}, item_type{type}, item_description{name} { }

			constexpr Item(const char *name, Value::Type type, const char *description)
				: item_name{name}, item_type{type}, item_description{description} { }

			inline const char *name() const noexcept {
				return item_name;
			}

			inline const Value::Type type() const noexcept {
				return item_type;
			}

			inline const char * description() const noexcept {
				return item_description;
			}
		};

		Schema() {
		}

		template<typename... Targs>
		Schema(Targs... Fargs) {
			append(Fargs...);
		}

		inline void append(const Item &item) {
			itens.push_back(item);
		}

		template<typename... Targs>
		inline void append(const Item &item, Targs... Fargs) {
			append(item);
			append(Fargs...);
		}

		inline auto begin() const noexcept {
			return itens.begin();
		}

		inline auto end() const noexcept {
			return itens.end();
		}

	private:
		std::vector<Item> itens;

	};

 }

