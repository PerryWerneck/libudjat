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

	File::List::List(const char *pattern) {

		WIN32_FIND_DATA FindFileData;
		memset(&FindFileData,0,sizeof(FindFileData));

		HANDLE hFind = FindFirstFile(pattern,&FindFileData);

		if (hFind == INVALID_HANDLE_VALUE) {
			throw Win32::Exception("Can't find file");
		}

		string path;

		const char * ptr[] = {
			strchr(pattern,'\\'),
			strchr(pattern,'/')
		};

		if(ptr[0] || ptr[1]) {

			size_t len = strlen(pattern);

			if(ptr[0]) {
				len = min(len,(size_t) (ptr[0]-pattern));
			}

			if(ptr[1]) {
				len = min(len,(size_t) (ptr[1]-pattern));
			}

			path.assign(pattern,len);

		}

		//cout << "PATH: [" << path << "]" << endl;

		try {

			do {

				string str(path);
				str += FindFileData.cFileName;
				//cout << "FILENAME: [" << FindFileData.cFileName << "]" << endl;
				this->push_back(str);

			} while (FindNextFile(hFind, &FindFileData) != 0);

			DWORD dwError = GetLastError();
			if(dwError != ERROR_NO_MORE_FILES) {
				throw Win32::Exception("Error reading files");
			}

		} catch(...) {

			FindClose(hFind);
			throw;

		}

		FindClose(hFind);

	}

	File::List::~List() {
	}

	void File::List::forEach(std::function<void (const char *filename)> call) {
		for(auto ix = begin(); ix != end(); ix++)  {
			call( (*ix).c_str());
		}
	}

 }
