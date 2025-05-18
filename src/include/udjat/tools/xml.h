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

 #include <udjat/defs.h>
 #include <pugixml.hpp>
 #include <udjat/defs.h>
 #include <functional>
 #include <cstdint>

 namespace Udjat {

	class String;
	class Quark;

	namespace XML {

		using Node = pugi::xml_node;
		using Attribute = pugi::xml_attribute;

		/// @brief XML document
		class Document : public pugi::xml_document {
		public:
			Document(const char *filename);

			/// @brief Load document, build objects.
			/// @return Timestamp for the next reload.
			time_t ObjectFactory() const;

		};

		/// @brief Test if attribute is 'true', parse URL is necessary.
		/// @param node Start node.
		/// @param attrname Attribute name.
		/// @param defvalue The default value.
		/// @return The attribute parsed as boolean.
		UDJAT_API bool test(const XML::Node &node, const char *attrname, bool defvalue = false);

		/// @brief Navigate on node options.
		UDJAT_API void options(const XML::Node &node, const std::function<void(const char *name, const char *value)> &call);

		/// @brief Search 'node' and up stream for 'attrname'.
		/// @param node Start node.
		/// @param attrname Attribute name.
		/// @return The attribute (empty if not found).
		UDJAT_API XML::Attribute AttributeFactory(const XML::Node &node, const char *attrname);

		/// @brief Search 'node' and up stream for 'attrname'.
		/// @param node Start node.
		/// @param attrname Attribute name.
		/// @param def default value if nullptr the attribute is required.
		/// @return The attribute value (def if not found).
		UDJAT_API const char * StringFactory(const XML::Node &node, const char *attrname, const char *def = "");

		/// @brief Search 'node' and up stream for 'attrname'.
		/// @param node Start node.
		/// @param attrname Attribute name.
		/// @param def default value if nullptr the attribute is required.
		/// @return Quark with attribute value or 'def' if not found.
		UDJAT_API const char * QuarkFactory(const XML::Node &node, const char *attrname, const char *def = "");

		/// @brief Navigate from document until callback returns true.
		/// @param node Start node.
		/// @param attrname Attribute name.
		/// @return Test result.
		/// @retval false if test function returned false in all nodes.
		/// @retval true if test function returned true.
		UDJAT_API bool for_each(const XML::Node &node, const char *attrname, const std::function<bool(const XML::Node &node)> &test);

		/// @brief Load default XML files.
		/// @param path Path for configuration file or directory.
		/// @return Timestamp for the next reload.
		/// @retval 0 if no reload is required.
		UDJAT_API time_t load(const char *path = nullptr);

		/// @brief Load xml options below node.
		UDJAT_API bool load(const XML::Node &node);
	
	}

	/// @brief Test common filter options.
	/// @return true if the node is valid.
	UDJAT_API bool is_allowed(const XML::Node &node);

	/// @brief Test reserved node names.
	/// @return true if the node is valid.
	UDJAT_API bool is_reserved(const XML::Node &node);

	/// @brief Expand, if possible, values ${} from attribute.
	UDJAT_API std::string expand(const XML::Node &node, const XML::Attribute &attribute, const char *def);

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

		/// @brief Convert string value to quark and return the stored value.
		const char * c_str(const char *def = "") const;

		/// @brief Select value from list.
		/// @return Index of the attribute value (or exception if not found).
		size_t select(const char *value, ...) __attribute__ ((sentinel));

		std::string to_string(const std::string &def) const;

		/// @brief Search XML tree for attribute.
		/// @param node the startup node.
		/// @param aname The required attribute node.
		/// @param vname The tag on <attribute> to get attribute value.
		/// @return The value tag from <attribute name=${aname} ${vname}=value /> or <node ${aname}=value /> or other standard searches.
	};

 }

