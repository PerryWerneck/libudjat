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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/string.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/logger.h>
 #include "private.h"

 namespace Udjat {

	Application::InstallLocation::operator bool() const {

		if(empty()) {
			return false;
		}

#ifdef DEBUG
		cout	<< "InstallLocation='" << c_str() << "'" << endl
				<< "ApplicationPath='" << Application::Path().c_str() << "'" << endl;
#endif
		return strcmp(c_str(),Application::Path().c_str()) == 0;

	}

	Application::InstallLocation::InstallLocation() {

		string path{"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"};
		path += Application::Name();

		debug("Searching for 'InstallLocation' in '",path,"'");

		static const DWORD options[] = { KEY_READ|KEY_WOW64_32KEY, KEY_READ|KEY_WOW64_64KEY };

		for(size_t ix = 0; ix < (sizeof(options)/sizeof(options[0])); ix++) {
			HKEY hKey;
			LSTATUS rc =
				RegOpenKeyEx(
					HKEY_LOCAL_MACHINE,
					TEXT(path.c_str()),
					0,
					options[ix],
					&hKey
				);

			if(rc == ERROR_SUCCESS) {
				Win32::Registry registry{hKey};
				if(registry.hasValue("InstallLocation")) {
					assign(registry.get("InstallLocation",""));

					if(at(size()-1) != '\\') {
						append("\\");
					}

					debug("InstallLocation='",c_str(),"'");
					return;

				}
			} else if(rc != ERROR_FILE_NOT_FOUND) {
				cerr << "win32\t" << Win32::Exception::format(path.c_str()) << endl;
			}
		}

		debug("No 'InstallLocation' registry key");

	}

	/*
	static void replace_path(Udjat::String &str, const char *key, REFKNOWNFOLDERID id) {

		const char *ptr = str.strcasestr(key);
		if(ptr) {
			str.replace(ptr - str.c_str(),strlen(key),Win32::KnownFolder{id});
		}

	}
	*/

	std::string PathFactory(REFKNOWNFOLDERID id, const char *subdir) {

		File::Path registry{Win32::Registry{"paths"}.get(subdir,"")};

		if(!registry.empty()) {
			return registry;
		}

		Win32::KnownFolder folder{id,subdir};
		File::Path::mkdir(folder.c_str());
		return folder;

	}

 }
