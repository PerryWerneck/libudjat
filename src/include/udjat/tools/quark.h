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

#ifndef QUARK_H_INCLUDED

	#define QUARK_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/xml.h>
	#include <cstring>
	#include <functional>
	#include <ostream>

	namespace Udjat {

		/// @brief Single instance string.
		class UDJAT_API Quark {
		private:
			class Controller;
			friend class Controller;

			const char *value;

		public:

			/// @brief Initialize Quark Engine.
			static void init();

			static Quark getFromStatic(const char *str);

			Quark() : value(nullptr) {}

			Quark(const char *str);
			Quark(const std::string &str);
			Quark(const Quark &src);
			Quark(const Quark *src);
			Quark(const pugi::xml_attribute &attribute);

			/// @brief Create quark from XML attribute
			/// @param node	XML node.
			/// @param Attribute name.
			/// @param def Default value.
			/// @param upsearch If true search the parent nodes.
			Quark(const XML::Node &node,const char *name,const char *def="",bool upsearch = true);

			Quark & operator=(const char *str);
			Quark & operator=(const std::string &str);
			Quark & operator=(const pugi::xml_attribute &attribute);

			Quark & operator=(const Quark &src) {
				value = src.value;
				return *this;
			}

			Quark & operator=(const Quark *src) {
				value = src->value;
				return *this;
			}

			const char * c_str() const;

			size_t hash() const;

			operator bool() const {
				return value != nullptr && *value;
			}

			bool operator==(const Quark &src) const {
				return this->value == src.value;
			}

			bool operator==(const char *str) const {
				return compare(str);
			}

			bool compare(const char *str) const {
				return strcmp(c_str(),str);
			}

			const Quark & set(const char *str);
			const Quark & set(const char *str, const std::function<const char * (const char *key)> translate);

#ifdef HAVE_PUGIXML
			const Quark & set(const XML::Node &node, const char *xml_attribute, bool upsearch = false);
			const Quark & set(const XML::Node &node, const char *xml_attribute, bool upsearch, const std::function<const char * (const char *key)> translate);
#endif // HAVE_PUGIXML

		};
	}

	#define I_(str) Udjat::Quark::getFromStatic(str)

	namespace std {

		template <>
		struct hash<Udjat::Quark> {
			inline size_t operator() (const Udjat::Quark &quark) const {
				return std::hash<std::string>{}(quark.c_str());
			}
		};

		inline ostream& operator<< (ostream& os, const Udjat::Quark &quark ) {
			return os << ((const char *)quark.c_str());
		}

	}

#endif // QUARK_H_INCLUDED
