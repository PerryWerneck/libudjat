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

	/*
	static void load(std::list<std::string> &files, const char *fpat, bool recursive) {

		if(!(fpat && *fpat)) {
			throw system_error(EINVAL,system_category(),"Empty file pattern");
		}

		string folder{fpat};
		for(char *ptr = (char *) folder.c_str();*ptr;ptr++) {
			if(*ptr == '/') {
				*ptr = '\\';
			}
		}

		// Split folder & mask

		if(folder[folder.size()-1] == '\\') {
			folder.resize(folder.size()-1);
		} else {
			throw system_error(ENOTSUP,system_category(),"Pattern based filter is not supported");

			// TODO: Use PathMatchSpec to match file pattern.

		}

		// Scan folder.
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile((folder + "\\*").c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			throw Win32::Exception(folder.c_str());
		}

		try {

			do {

				if(FindFileData.cFileName[0] == '.') {
					continue;
				}

				string filename = folder + "\\" + FindFileData.cFileName;

				DWORD attr = GetFileAttributes(filename.c_str());
				if(attr == INVALID_FILE_ATTRIBUTES) {
					throw Win32::Exception(filename);
				}

				if(recursive && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
#ifdef DEBUG
					cout << "Found folder '" << filename << "'" << endl;
#endif // DEBUG
					load(files, (filename + "\\").c_str(),true);

				} else {
#ifdef DEBUG
					cout << "Found file '" << filename << "'" << endl;
#endif // DEBUG
					files.push_back(filename);
				}

			} while(FindNextFile(hFind, &FindFileData) != 0);

		} catch(...) {

			FindClose(hFind);
			throw;

		}

		FindClose(hFind);
	}

	File::List::List(const char *fpat, bool recursive) {
		load(*this,fpat,recursive);
	}
	*/

	File::List::List(const char *path, bool recursive) : List(path,"*",recursive) {
	}

	File::List::List(const char *path, const char *pattern, bool recursive) {

		File::Path{path}.for_each([this,pattern](const File::Path &filename){
			if(filename.match(pattern)) {
				emplace_back(filename);
			}
			return false;
		},recursive);

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
