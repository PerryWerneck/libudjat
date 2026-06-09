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
 #include <functional>

 namespace Udjat {

	/// @brief Abstract class for object property parser.
	class UDJAT_API Properties {
	public:
		Properties() = default;
		virtual ~Properties() = default;

		/// @brief Get the property node name, usually the value from XML.
		/// @return The property node name.
		virtual const char * node_name() const noexcept;

		/// @brief Build constant from 'name' attribute, fallback to node_name() if cant find the attribute.
		/// @return The property name as constant string.
		const char * NameFactory() const noexcept;

		/// @brief Build a string from attrname.
		/// @param attrname The attribute name.
		/// @param def default value if nullptr the attribute is required.
		/// @return The property value of def if not found.
		virtual const String get(const char *attrname, const char *def = "") const;
		
		/// @brief On XML property get the text inside the node.
		/// @return The child value.
		virtual String child_value() const;

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

