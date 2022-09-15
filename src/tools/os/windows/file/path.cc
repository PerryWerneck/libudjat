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

 #include "private.h"
 #include <shlwapi.h>
 #include <dirent.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/path.h>

 namespace Udjat {

	Win32::Path::Path(const char *pathname) : std::string(pathname) {
		for(char *ptr = (char *) c_str();*ptr;ptr++) {
			if(*ptr == '/') {
				*ptr = '\\';
			}
		}

		if(at(size()-1) == '\\') {
			resize(size()-1);
		}
	}

	bool Win32::Path::dir(const char *pathname) {
		DWORD attr = GetFileAttributes(pathname);
		if(attr == INVALID_FILE_ATTRIBUTES) {
			throw Win32::Exception(pathname);
		}
		return attr & FILE_ATTRIBUTE_DIRECTORY;
	}

	File::Path::Path(int UDJAT_UNUSED(fd)) {
		throw system_error(ENOTSUP,system_category(),"Not available on windows");
	}

	void File::Path::save(const char *filename, const char *contents) {
		// Get file information.
		throw system_error(ENOTSUP,system_category(),"Not available on windows");
	}

	bool File::Path::for_each(const char *pathname, const std::function<bool (const char *name, const Stat &stat)> &call) {

		Win32::Path path{pathname};

		DIR *dir = opendir(path.c_str());

		if(!dir) {
			throw system_error(ENOENT,system_category(),path);
		}

		bool rc = true;

		try {

			struct dirent *de;
			while(rc && (de = readdir(dir)) != NULL) {

				if(de->d_name[0] == '.') {
					continue;
				}

				string filename{path};
				filename += "\\";
				filename += de->d_name;

				Stat st;
				if(stat(filename.c_str(),&st) == -1) {
					cerr << filename << ": " << strerror(errno) << endl;
					continue;
				}

				rc = call(filename.c_str(), st);

			}


		} catch(...) {

			closedir(dir);
			throw;

		}

		closedir(dir);

		return rc;

	}

	bool File::Path::for_each(const char *pathname, const char *pattern, bool recursive, std::function<bool (const char *)> call) {

		Win32::Path path{pathname};

		DIR *dir = opendir(path.c_str());

		if(!dir) {
			throw system_error(ENOENT,system_category(),path);
		}

		bool rc = true;

		try {

			struct dirent *de;
			while(rc && (de = readdir(dir)) != NULL) {

				if(de->d_name[0] == '.') {
					continue;
				}

				string filename{path};
				filename += "\\";
				filename += de->d_name;

				if(recursive && Win32::Path::dir(filename.c_str())) {

					rc = for_each(filename.c_str(), pattern, recursive, call);

				} else if(PathMatchSpec(de->d_name,pattern)) {

#ifdef DEBUG
					cout << "found '" << filename << "'" << endl;
#endif // DEBUG

					rc = call(filename.c_str());

				}
#ifdef DEBUG
				else {
					cout << "Not found '" << filename << "'" << endl;
				}
#endif // DEBUG

			}


		} catch(...) {

			closedir(dir);
			throw;

		}

		closedir(dir);

		return rc;
	}

}

