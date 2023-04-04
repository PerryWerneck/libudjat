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

 #pragma once
 #include <config.h>
 #include <udjat/defs.h>
 #include <string>
 #include <shlobj.h>
 #include <udjat/tools/file.h>

 namespace Udjat {

	namespace Win32 {

		UDJAT_PRIVATE std::string PathFactory(REFKNOWNFOLDERID id, const char *subdir);

		/// @brief Windows special folder.
		/// @see https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid
		/// @see https://gitlab.gnome.org/GNOME/glib/blob/main/glib/gutils.c
		class UDJAT_PRIVATE KnownFolder : public File::Path {
		public:
			KnownFolder(REFKNOWNFOLDERID known_folder_guid_ptr);
			KnownFolder(REFKNOWNFOLDERID known_folder_guid_ptr, const char *subdir);
		};

	}

 }

 /// @brief Get Win32 special folder.
 // std::string win32_special_folder(REFKNOWNFOLDERID known_folder_guid_ptr);
 // std::string win32_special_folder(REFKNOWNFOLDERID known_folder_guid_ptr, const char *subdir);

