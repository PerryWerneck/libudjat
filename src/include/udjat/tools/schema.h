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
 #include <udjat/tools/variant.h>
 #include <vector>

 namespace Udjat {

	/// @brief Encapsulates the structural layout and validation rules used to parse, verify, and serialize structured data formats.
	class UDJAT_API Schema {
	public:

		/// @brief Schema capabilites.
		enum Capabilities : uint8_t {
			NoCapabilities		= 0x0,
			Enumerable			= 0x1,				///< @brief Get on '/' enumerate objects.
		};

		Capabilities caps = Schema::NoCapabilities;

		enum Type : uint8_t {
			Path		= Variant::ObjectPath,		///< @brief Object path as string.
			String		= Variant::String,			///< @brief UTF-8 string value.
			Timestamp	= Variant::Timestamp,		///< @brief Timestamp value.
			Signed		= Variant::Signed,			///< @brief Signed integer value.
			Unsigned	= Variant::Unsigned,		///< @brief Unsigned integer value.
			Double		= Variant::Real,			///< @brief Double value.
			Float		= Variant::Real,			///< @brief Float value.
			Boolean		= Variant::Boolean,			///< @brief Boolean value.
			Icon		= Variant::Icon,			///< @brief Icon name.
			Url			= Variant::Url,				///< @brief URL.
			State		= Variant::State,			///< @brief Agent state (Ready=✓, Warning=⚠, error=✘, etc.)
			Percent		= Variant::Fraction,		///< @brief Percent value (Float from 0.0 to 1.0).
		};

		template <typename T>
		constexpr static Type TypeFactory() {
			return Type::String;
		}

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

			inline bool operator==(const Type type) const noexcept {
				return item_type == type;
			}

		protected:
			friend class Schema;
			
			const char *item_name;			//< @brief The item name.
			Type item_type;					///< @brief The item type.
			const char *item_description;	///< @brief The item description.

		};

		Schema() = default;

		void add(const Item &item);

		inline void add(const Schema::Capabilities cap) noexcept {
			caps = (Schema::Capabilities) (caps|cap);	
		}

		template<typename... Targs>
		inline void add(const Item &item, Targs... Fargs) {
			add(item);
			add(Fargs...);
		}

		template<typename... Targs>
		inline void add(const Capabilities cap, Targs... Fargs) {
			add(cap);
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

	class UDJAT_API InputSchema : public Schema {
	public:
		InputSchema() = default;

		template<typename... Targs>
		InputSchema(Targs... Fargs) {
			add(Fargs...);
		}


	};

	class UDJAT_API OutputSchema : public Schema {
	public:

		/// @brief Template name, for http outputs.
		const char *template_name = nullptr;

		OutputSchema() = default;

		template<typename... Targs>
		OutputSchema(Targs... Fargs) {
			add(Fargs...);
		}

	};

	class HTTPSchema;

	template <>
	constexpr Schema::Type Schema::TypeFactory<std::string>() {
		return Type::String;
	}

	template <>
	constexpr Schema::Type Schema::TypeFactory<const char *>() {
		return Type::String;
	}

	template <>
	constexpr Schema::Type Schema::TypeFactory<TimeStamp>() {
		return Type::Timestamp;
	}

	template <>
	constexpr Schema::Type Schema::TypeFactory<int>() {
		return Type::Signed;
	}

	template <>
	constexpr Schema::Type Schema::TypeFactory<unsigned int>() {
		return Type::Unsigned;
	}

	template <>
	constexpr Schema::Type Schema::TypeFactory<float>() {
		return Type::Float;
	}

	template <>
	constexpr Schema::Type Schema::TypeFactory<double>() {
		return Type::Double;
	}

	template <>
	constexpr Schema::Type Schema::TypeFactory<bool>() {
		return Type::Boolean;
	}

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Schema::Type type);

}