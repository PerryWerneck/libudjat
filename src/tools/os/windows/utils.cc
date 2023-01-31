/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2022 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/win32/cleanup.h>
 #include <udjat/tools/application.h>
 #include <private/win32.h>
 #include <stdexcept>

 using namespace std;
 using namespace Udjat;

 UDJAT_API void udjat_autoptr_cleanup_HANDLE(HANDLE *handle) {
	if(*handle) {
		CloseHandle(*handle);
		*handle = 0;
	}
 }

 Udjat::Win32::KnownFolder::KnownFolder(REFKNOWNFOLDERID known_folder_guid_ptr) {

	PWSTR wcp = NULL;
	string result;

	HRESULT hr = SHGetKnownFolderPath(known_folder_guid_ptr, 0, NULL, &wcp);

	try {

		if (SUCCEEDED(hr)) {

			size_t len = wcslen(wcp) * 2;
			char buffer[len+1];

			wcstombs(buffer,wcp,len);

			assign(buffer);

		} else {
			throw runtime_error("Can't get known folder path");
		}

	} catch(...) {
		CoTaskMemFree (wcp);
		throw;
	}

	CoTaskMemFree (wcp);

	append("\\");

 }

 Udjat::Win32::KnownFolder::KnownFolder(REFKNOWNFOLDERID known_folder_guid_ptr, const char *subdir) : KnownFolder{known_folder_guid_ptr} {

	append(Application::Name());
	mkdir();

	append("\\");
	append(subdir);
	mkdir();

	append("\\");

 }

