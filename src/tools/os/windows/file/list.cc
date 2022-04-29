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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/file.h>
 #include <udjat/win32/exception.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	File::List::List(const char *filepattern) {

		if(!(filepattern && *filepattern)) {
			throw system_error(EINVAL,system_category(),"Empty file pattern");
		}

		string pattern{filepattern};
		for(char *ptr = (char *) pattern.c_str();*ptr;ptr++) {
			if(*ptr == '/') {
				*ptr = '\\';
			}
		}

		if(pattern[pattern.size()-1] == '\\') {
			pattern += "*.*";
		}

#ifdef DEBUG
		cout << "Searching for '" << pattern << "'" << endl;
#endif // DEBUG

		WIN32_FIND_DATA FindFileData;
		memset(&FindFileData,0,sizeof(FindFileData));

		HANDLE hFind = FindFirstFile(pattern.c_str(),&FindFileData);

		if (hFind == INVALID_HANDLE_VALUE) {
			throw Win32::Exception(string{"Can't find '"} + pattern + "'");
		}

		string path{pattern};

		size_t pos = path.rfind('\\');
		if(pos != string::npos) {
			path.resize(pos);
		}

		try {

			do {

				if(FindFileData.cFileName[0] != '.') {
					string str(path);
					str += '\\';
					str += FindFileData.cFileName;
					this->push_back(str);
#ifdef DEBUG
					cout << "Found '" << str << "'" << endl;
#endif // DEBUG
				}

			} while (FindNextFile(hFind, &FindFileData) != 0);

			DWORD dwError = GetLastError();
			if(dwError != ERROR_NO_MORE_FILES) {
				throw Win32::Exception("Error reading files",dwError);
			}

		} catch(...) {

			FindClose(hFind);
			throw;

		}

		FindClose(hFind);

	}

	File::List::~List() {
	}

	bool File::List::for_each(std::function<bool (const char *filename)> call) {
		for(auto ix = begin(); ix != end(); ix++)  {
			if(!call( (*ix).c_str())) {
				return false;
			}
		}
		return true;
	}

 }
