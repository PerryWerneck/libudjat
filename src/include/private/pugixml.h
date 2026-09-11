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

 #pragma once

 #include <udjat/defs.h>
 #include <pugixml.hpp>
 #include <udjat/tools/properties.h>
 #include <udjat/defs.h>
 #include <functional>
 #include <cstdint>
 #include <cstring>
 #include <udjat/tools/properties.h>

 namespace Udjat {

	class String;
	class Quark;

	namespace XML {

		class UDJAT_API Node : public pugi::xml_node, public Properties {
		public:
			Node() = default;

			Node(const pugi::xml_node &node) : pugi::xml_node{node} {
			}

			~Node() override;

        	bool load(const char *filename) override;

			Properties parent() const noexcept override;

			Node child(const char *name) const;

			Node next_sibling(const char *name) const;	

			bool reserved() const noexcept override;

			bool allowed() const noexcept override;

			const char *node_name() const noexcept override;

			String path() const noexcept override;

			bool contains(const char *name, bool parent = false) const noexcept override;

			bool has_child(const char *name) const noexcept override;

			const String get(const char *attrname, const char *def = "") const override;
			bool get(const char *attrname, const bool def) const override;
			double get(const char *attrname, const double def) const override;
			float get(const char *attrname, const float def) const override;
			int get(const char *attrname, const int def) const override;
			unsigned int get(const char *attrname, const unsigned int def) const override;
			unsigned long get(const char *attrname, const unsigned long def) const override;

			// template <typename T>
			// inline T get(const char *groupname, const char *attrname, const T def) const {
			// 	if(contains(attrname)) {
			// 		return get(attrname,def);
			// 	}
			// 	return Config::get(groupname,attrname,def);
			// }
			
			String child_value() const override;
			String child_value(const char *attrname, const char *def) const override;

			bool for_each_child(const char *tagname, const std::function<bool(const Properties &property)> &call) const override;

			bool for_each_child(const std::function<bool(const Properties &property)> &call) const override;

			bool for_each_child(const char *tagname, const char *group, const std::function<bool(const Udjat::Properties &property)> &call) const override;

			bool for_each_attribute(const char *attrname, const std::function<bool(const Udjat::Properties &props)> &test) const override;

		};

		using Attribute = pugi::xml_attribute;

		/// @brief Load multiple child nodes into a container.
		/// @details This function loads all child nodes with the given name into the provided container.
		/// @tparam C 
		/// @param node Parent node.
		/// @param attrname XML attribute name for child nodes.
		/// @param container The container to load nodes into.
		template <class C>
		inline void load(const pugi::xml_node &node, const char *attrname, C &container) {
			for(auto child = node.child(attrname); child; child = child.next_sibling(attrname)) {
				container.emplace_back(child);
			}
		}

		/// @brief XML document
		class UDJAT_API Document : public pugi::xml_document {
		public:
			Document(const char *filename);
			Document(const char *data, size_t size);

			/// Copy document node to the given node.
			/// @param node Node to copy the document root.
			Node & copy_to(Node &node) const;

			/// @brief Load document, build objects.
			/// @return Timestamp for the next reload.
			time_t parse() const;

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

		/// @brief Load default XML files.
		/// @param path Path for configuration file or directory.
		/// @return Timestamp for the next reload.
		/// @retval 0 if no reload is required.
		UDJAT_API time_t parse(const char *path = nullptr);

		/// @brief Load xml options for node.
		/// @param node XML node to parse.
		/// @param recursive If true, parse children nodes too.
		/// @return true if the node was parsed or should be ignored by the caller.
		UDJAT_API bool parse(const pugi::xml_node &node, bool recursive = false);

		/// @brief Load options for node children, doesn't parse the node itself.
		/// @details This function is used to parse the children of a node, it doesn't parse or even check the node itself.
		/// @param node root XML node.
		/// @param recursive If true, parse children nodes too.
		/// @return true if any child node was parsed.
		/// @retval false if no child node was parsed
		UDJAT_API bool parse_children(const pugi::xml_node &node, bool recursive = false);
		
	}

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

	template <typename T>
	inline T from_xml(const XML::Node &node, const T def, const char *attrname = "value") {
		throw std::logic_error("No XML converter for this data format");
	}

	template <>
	inline int from_xml<int>(const XML::Node &node, const int def, const char *attrname) {
		return node.pugi::xml_node::attribute(attrname).as_int(def);
	}

	template <>
	inline unsigned int from_xml<unsigned int>(const XML::Node &node, const unsigned int def, const char *attrname) {
		return node.pugi::xml_node::attribute(attrname).as_uint(def);
	}

	template <>
	inline short from_xml<short>(const XML::Node &node, const short def, const char *attrname) {
		return (short) node.pugi::xml_node::attribute(attrname).as_int(def);
	}

	template <>
	inline unsigned short from_xml<unsigned short>(const XML::Node &node, const unsigned short def, const char *attrname) {
		return (unsigned short) node.pugi::xml_node::attribute(attrname).as_int(def);
	}

	template <>
	inline long from_xml<long>(const XML::Node &node, const long def, const char *attrname) {
		return (long) node.pugi::xml_node::attribute(attrname).as_int(def);
	}

	template <>
	inline unsigned long from_xml<unsigned long>(const XML::Node &node, const unsigned long def, const char *attrname) {
		return (unsigned long) node.pugi::xml_node::attribute(attrname).as_uint(def);
	}

	template <>
	inline float from_xml<float>(const XML::Node &node, const float def, const char *attrname) {
		return node.pugi::xml_node::attribute(attrname).as_float(def);
	}

	template <>
	inline double from_xml<double>(const XML::Node &node, const double def, const char *attrname) {
		return node.pugi::xml_node::attribute(attrname).as_double(def);
	}

 }

