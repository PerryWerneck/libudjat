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

/**
 * @file src/include/udjat/tools/xml.h
 *
 * @brief Declare the udjat XML classes and tools
 *
 * @author perry.werneck@gmail.com
 *
 */

 #pragma once

 #include <pugixml.hpp>
 #include <udjat/defs.h>
 #include <functional>

 namespace Udjat {

	namespace XML {
		using Node = pugi::xml_node;
		using Attribute = pugi::xml_attribute;
	}

	/// @brief Test common filter options.
	/// @return true if the node is valid.
	UDJAT_API bool is_allowed(const XML::Node &node);

	/// @brief Expand, if possible, values ${} from attribute.
	UDJAT_API std::string expand(const XML::Node &node, const XML::Attribute &attribute, const char *def);

	/// @brief Scan for xml documents.
	/// @param path File name or path to scan for XML documents.
	/// @param call method to call in every filename.
	/// @return true if the parse was ok.
	UDJAT_API bool for_each(const char *path, const std::function<void(const char *filename, const pugi::xml_document &document)> &call);

	/// @brief Wrapper for XML attribute
	class UDJAT_API Attribute : public XML::Attribute {
	private:
		std::string value;

	public:
		Attribute(const XML::Node &node, const char *name, const char *upsearch);
		Attribute(const XML::Node &node, const char *name, bool upsearch);
		Attribute(const XML::Node &node, const char *name);

		operator uint32_t() const {
			return as_uint();
		}

		operator int32_t() const {
			return as_int();
		}

		operator bool() const {
			return as_bool();
		}

		/// @brief Return string value as quark.
		//Quark as_quark(const char *def = "") const;

		/// @brief Convert string value to quark and return the stored value.
		const char * c_str(const char *def = "") const;

		/// @brief Select value from list.
		/// @return Index of the attribute value (or exception if not found).
		size_t select(const char *value, ...) __attribute__ ((sentinel));

		std::string to_string(const std::string &def) const;

	};

 }

