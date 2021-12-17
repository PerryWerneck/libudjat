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

 namespace Udjat {

	namespace Win32 {

		class UDJAT_API Registry {
		protected:
			HKEY hKey = 0;

		public:
			Registry(const char *path = nullptr, bool write = false);
			Registry(HKEY hParent, const char *path = nullptr, bool write = false);

			~Registry();

			std::string get(const char *name, const char *def) const;

		};

	}

 }


