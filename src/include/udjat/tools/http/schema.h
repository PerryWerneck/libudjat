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
 #include <udjat/tools/http/method.h>
 #include <udjat/authentication.h>
 #include <vector>

 namespace Udjat {

	/// @brief Encapsulates the structural layout and validation rules used to parse, verify, and serialize structured data formats.
	class UDJAT_API Schema::Method {
	public:

		class UDJAT_API Item {
		public:

			constexpr Item(const HTTP::Method method)
				: item_method{method} { }

			constexpr Item(const HTTP::Method method, const Authentication::Role role)
				: item_method{method}, item_role{role} { }

			constexpr Item(const HTTP::Method method, const char *description)
				: item_method{method}, item_description{description} { }

			constexpr Item(const char *name, const HTTP::Method method, const Authentication::Role role, char *description)
				: item_method{method}, item_role{role} { }

			inline const HTTP::Method method() const noexcept {
				return item_method;
			}

			inline const Authentication::Role role() const noexcept {
				return item_role;
			}

			inline const char * description() const noexcept {
				return item_description;
			}

		protected:		
			friend class Method;
				
			HTTP::Method item_method = HTTP::Get;					///< @brief The item method.
			Authentication::Role item_role = Authentication::None;	///< @brief The required role.
			const char *item_description = "";						///< @brief The item description.

		};

		Method() = default;

		void add(const Item &item);

		template<typename... Targs>
		inline void add(const Item &item, Targs... Fargs) {
			add(item);
			add(Fargs...);
		}

#if __cplusplus >= 201703L
		inline auto begin() const noexcept {
			return itens.begin();
		}

		inline auto end() const noexcept {
			return itens.end();
		}
#else
		inline std::vector<Item>::const_iterator begin() const noexcept {
			return itens.begin();
		}

		inline std::vector<Item>::const_iterator end() const noexcept {
			return itens.end();
		}
#endif

		inline void clear() {
			itens.clear();
		}

	private:
		std::vector<Item> itens;

	};

 }

