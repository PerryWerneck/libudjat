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

		enum Type : uint8_t {
			String		= Value::String,			///< @brief UTF-8 string value.
			Timestamp	= Value::Timestamp,			///< @brief Timestamp value.
			Signed		= Value::Signed,			///< @brief Signed integer value.
			Unsigned	= Value::Unsigned,			///< @brief Unsigned integer value.
			Double		= Value::Real,				///< @brief Double value.
			Float		= Value::Real,				///< @brief Float value.
			Boolean		= Value::Boolean,			///< @brief Boolean value.
			Icon		= Value::Icon,				///< @brief Icon name.
			Url			= Value::Url,				///< @brief URL.
			State		= Value::State,				///< @brief Agent state (Ready=✓, Warning=⚠, error=✘, etc.)
			Percent		= Value::Fraction,			///< @brief Percent value (Float from 0.0 to 1.0).
		};

		/// @brief Template name, for http outputs.
		const char *template_name = nullptr;

		class UDJAT_API Item {
		public:

			constexpr Item(const char *name, const Type type)
				: item_name{name}, item_type{type}, item_description{name} { }

			constexpr Item(const char *name, const Type type, const char *description)
				: item_name{name}, item_type{type}, item_description{description} { }

			inline const char *name() const noexcept {
				return item_name;
			}

			inline const Type type() const noexcept {
				return item_type;
			}

			inline const char * description() const noexcept {
				return item_description;
			}

		protected:
			friend class Schema;
			
			const char *item_name;			//< @brief The item name.
			Type item_type;					///< @brief The item type.
			const char *item_description;	///< @brief The item description.

		};

		Schema() {
		}

		template<typename... Targs>
		Schema(Targs... Fargs) {
			append(Fargs...);
		}

		void append(const Item &item);

		template<typename... Targs>
		inline void append(const Item &item, Targs... Fargs) {
			append(item);
			append(Fargs...);
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

