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
#include <list>
#include <udjat/tools/string.h>

namespace Udjat {

	namespace File {

		/// @brief Directory contents.
		class UDJAT_API List : public std::list<String> {
		public:
			List(const char *path, const char *pattern, bool recursive=false);
			List(const char *path, bool recursive=false);

			List(const std::string &pattern, bool recursive=false) : List(pattern.c_str(),recursive) {
			}

			~List();

			/// @brief Navigate for all files until lambda returns 'false'.
			/// @return true if the lambda doesnt returns 'false' on any file.
			bool for_each(std::function<bool (const char *filename)> call);

		};

	}

}
