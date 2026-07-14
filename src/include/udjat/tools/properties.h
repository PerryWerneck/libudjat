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
 #include <cstring>
 #include <functional>

 namespace Udjat {

	/// @brief Abstract class for object property parser.
	class UDJAT_API Properties {
	public:

		/// @brief Properties parser, used to parse Properties.
		/// @details This class is used to parse property definitions and build objects from them.
		class UDJAT_API ObjectBuilder {
		private:
			const char *builder_name = nullptr;

		public:
			ObjectBuilder(const char *name);
			virtual ~ObjectBuilder();

#if __cplusplus >= 202002L
			inline auto operator <=>(const char *name) const noexcept {
				return strcasecmp(name,builder_name);
			}
#else
			inline bool operator==(const char *name) const noexcept {
				return strcasecmp(name,builder_name) == 0;
			}
#endif

			/// @brief Build object from properties.
			/// @param props The property definitions to parse.
			/// @return true if the properties were parsed and should be ignored by the caller.
			virtual bool build(const Properties &props) = 0;

			inline const char *c_str() const noexcept {
				return builder_name;
			}

			inline const char *name() const noexcept {
				return builder_name;
			}

		};

		Properties() = default;
		virtual ~Properties() = default;

#if __cplusplus >= 202002L
			inline auto operator <=>(const char *name) const noexcept {
				return strcasecmp(name,node_name());
			}
#else
			inline bool operator==(const char *name) const noexcept {
				return strcasecmp(name,node_name()) == 0;
			}
#endif

		/// @brief Parse properties, build objects.
		static bool build(const Properties &props);

		/// @brief Check if it's a reserved tag.
		/// @return true if this is a reserved tag and should be ignored by factories.
		virtual bool reserved() const noexcept;

		/// @brief Check if property contains a value.
		/// @param name The value name.
		/// @param up If true scan uper nodes.
		/// @return true if the properties has the named value.
		virtual bool contains(const char *name, bool parent = false) const noexcept;

		virtual bool has_child(const char *name) const noexcept;

		/// @brief Check if this property is allowed
		virtual bool allowed() const noexcept;

		/// @brief Get the property node name, usually the value from XML.
		/// @return The property node name.
		virtual const char * node_name() const noexcept;

		/// @brief Get properties path (for messages)
		/// @return The properties path or 'properties' if object doesnt have a path.
		virtual String path() const noexcept;

		/// @brief Build constant from 'name' attribute, fallback to node_name() if cant find the attribute.
		/// @return The property name as constant string.
		const char * NameFactory() const noexcept;

		/// @brief Build a string from attrname.
		/// @param attrname The attribute name.
		/// @param def default value if nullptr the attribute is required.
		/// @return The property value of def if not found.
		virtual const String get(const char *attrname, const char *def = "") const;

		virtual bool get(const char *attrname, const bool def) const;
		virtual double get(const char *attrname, const double def) const;
		virtual float get(const char *attrname, const float def) const;
		virtual int get(const char *attrname, const int def) const;
		virtual unsigned int get(const char *attrname, const unsigned int def) const;
		
		/// @brief Get attribute with fallback to configuration file.
		/// @param groupname The group on the configuration file to search.
		/// @param attrname The attribute name.
		/// @param def The default value if the attribute doesnt exist on properties and config file.
		/// @return The value found or def if not exist.
		String get(const char *groupname, const char *attrname, const char * def) const;
		bool get(const char *groupname, const char *attrname, const bool def) const;
		double get(const char *groupname, const char *attrname, const double def) const;
		float get(const char *groupname, const char *attrname, const float def) const;
		int get(const char *groupname, const char *attrname, const int def) const;
		unsigned int get(const char *groupname, const char *attrname, const unsigned int def) const;

		/// @brief On XML property get the text inside the node.
		/// @return The child value.
		virtual String child_value() const;

		/// @brief Get the text insid an attribute.
		/// @param attrname The attribute name.
		/// @param def The default value if not found.
		/// @return The contents of the attribute or 'def' if not found.
		virtual String child_value(const char *attrname, const char *def) const;

		String operator[](const char *attrname) const;

		/// @brief Enumerate children by attribute name.https://x.com/evandroratho/status/2064074016793481270
		/// @param attrname The attribute name.
		/// @param call Method to callback.
		/// @return Test result.
		/// @retval false if callback function returned false in all children.
		/// @retval true if callback function returned true.
		virtual bool for_each_child(const std::function<bool(const Properties &property)> &call) const;

		/// @brief Enumerate children by name.
		/// @param attrname The children name (usually the XML tag).
		/// @param call Method to callback.
		/// @return Test result.
		/// @retval false if callback function returned false in all children.
		/// @retval true if callback function returned true.
		virtual bool for_each_child(const char *tagname, const std::function<bool(const Properties &property)> &call) const;

		/// @brief Navigate thru XML nodes, including groups.
		/// @param node The XML node to start search.
		/// @param tagname The child node name.
		/// @param group The child group node name, usually the plural of name (optional).
		/// @param handler The handler for children.
		virtual bool for_each_child(const char *tagname, const char *group, const std::function<bool(const Udjat::Properties &property)> &call) const;

		/// @brief Navigate from document until callback returns true.
		/// @param node Start node.
		/// @param attrname Attribute name.
		/// @return Test result.
		/// @retval false if test function returned false in all nodes.
		/// @retval true if test function returned true.
		virtual bool for_each_attribute(const char *attrname, const std::function<bool(const Udjat::Properties &props)> &test) const;
			
	};

 }

