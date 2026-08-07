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
 #include <udjat/tools/http/status.h>
 #include <vector>

 namespace Udjat {

	/// @brief Encapsulates the structural layout and validation rules used to parse, verify, and serialize structured data formats.
	namespace Schema {

		class Method;
		class Properties;

		enum Type : uint8_t {
			ObjectPath	= Variant::ObjectPath,		///< @brief Object path as string.
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
			friend class Properties;

			friend class Schema;
			
			const char *item_name;			//< @brief The item name.
			Type item_type;					///< @brief The item type.
			const char *item_description;	///< @brief The item description.

		};

		/// @brief Schema Input/Output properties.
		class UDJAT_API Properties {
		protected:
			std::vector<Item> itens;

		public:
			Properties() = default;

			void add(const Item &item);

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

			inline bool empty() {
				return itens.empty();
			}

			inline size_t size() {
				return itens.size();
			}

		};

		/// @brief Schema inputs
		class UDJAT_API Input : public Properties {
		public:

			enum Options : uint8_t {
				NoOptions 	= 0,		///< @brief Input has no options.
				AllowRoot	= 1			///< @brief Input allow '/' as path.
			} options = NoOptions;

			Input() = default;

			inline void add(const Item &item) {
				Properties::add(item);
			}

			inline void add(const Options opt) noexcept {
				options = (Options) (options|opt);	
			}

			template<typename... Targs>
			inline void add(const Item &item, Targs... Fargs) {
				add(item);
				add(Fargs...);
			}

			template<typename... Targs>
			inline void add(const Options opt, Targs... Fargs) {
				add(opt);
				add(Fargs...);
			}

			/// @brief Validate if variant has all the required fields.
			/// @param request The variant with the values.
			/// @param response Status to receive the response messages and error code.
			/// @return true if the variant contains all required inputs.
			// bool validate(const Variant &request, HTTP::Status &response) const noexcept;
			bool validate(const Variant &request, HTTP::Status &response) const noexcept;

		};

		/// @brief Schema outputs
		class UDJAT_API Output : public Properties {
		public:
			const char *template_name = nullptr;

			enum Options : uint8_t {
				NoOptions 	= 0,		///< @brief Input has no options.
				Enumerable	= 1			///< @brief Get on '/' enumerate the object itens.
			} options = NoOptions;

			Output() = default;

			inline void add(const Item &item) {
				Properties::add(item);
			}

			inline void add(const Options opt) noexcept {
				options = (Options) (options|opt);	
			}

			template<typename... Targs>
			inline void add(const Item &item, Targs... Fargs) {
				add(item);
				add(Fargs...);
			}

			template<typename... Targs>
			inline void add(const Options opt, Targs... Fargs) {
				add(opt);
				add(Fargs...);
			}


		};

	}

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