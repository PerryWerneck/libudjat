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

		virtual const char *name() const noexcept;

		/// @brief Build a string from attrname.
		/// @param attrname The attribute name.
		/// @param def Default value if the attribute wasnt found;
		/// @return The property value of def if not found.
		virtual const String get(const char *attrname, const char *def = "") const;
		
		/// @brief On XML property get the text inside the node.
		/// @return The child value.
		virtual String child_value() const;

		String operator[](const char *attrname) const;

		/// @brief Enumerate children by name.
		/// @param attrname The children name.
		/// @param call Method to callback.
		/// @return Test result.
		/// @retval false if callback function returned false in all children.
		/// @retval true if callback function returned true.
		virtual bool for_each(const char *name, const std::function<bool(const Properties &property)> &call) const;

		/// @brief Enumerate children by attribute name.
		/// @param attrname The attribute name.
		/// @param call Method to callback.
		/// @return Test result.
		/// @retval false if callback function returned false in all children.
		/// @retval true if callback function returned true.
		virtual bool for_each(const std::function<bool(const Properties &property)> &call) const;

	};

 }

