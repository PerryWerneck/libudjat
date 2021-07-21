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
 * @file src/include/udjat/agent.h
 *
 * @brief Declare the agent classes
 *
 * @author perry.werneck@gmail.com
 *
 */

#ifndef UDJAT_TOOLS_XML_H_INCLUDED

	#define UDJAT_TOOLS_XML_H_INCLUDED

	#include <pugixml.hpp>
	#include <udjat/defs.h>
	#include <udjat/tools/xml.h>
	#include <udjat/tools/quark.h>
	#include <string>

	namespace Udjat {

		/// @brief Expand, if possible, values ${} from str.
		std::string expand(const pugi::xml_node &node, const char *str);

		/// @brief Wrapper for XML attribute
		class UDJAT_API Attribute : public pugi::xml_attribute {
		private:
			std::string str;

		public:
			Attribute(const pugi::xml_node &node, const char *name, bool upsearch = true);

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
			Quark as_quark(const char *def = "") const;

			/// @brief Convert string value to quark and return the stored value.
			const char * c_str(const char *def = "") const;


			std::string to_string(const std::string &def) const;

		};

	}


#endif // UDJAT_TOOLS_XML_H_INCLUDED
