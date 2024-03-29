/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Declare icon file name.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <string>
 #include <ostream>

 namespace Udjat {

	class UDJAT_API Icon : public std::string {
	public:
		Icon(const char *name) : std::string{name} {
		}

		Icon(const std::string &name) : std::string{name} {
		}

		/// @brief Get icon filename.
		std::string filename() const;

		inline operator bool() const {
			return !empty();
		}

	};

 }

 namespace std {

	inline const char * to_string(const Udjat::Icon &icon) {
		return icon.c_str();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Icon &icon ) {
		return os << ((const char *) icon.c_str());
	}

 }
