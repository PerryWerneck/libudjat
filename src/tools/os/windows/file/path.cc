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
 #include <iostream>
 #include <udjat/tools/logger.h>

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

	bool File::Path::dir(const char *pathname) {
		DWORD attr = GetFileAttributes(pathname);
		if(attr == INVALID_FILE_ATTRIBUTES) {
			throw Win32::Exception(pathname);
		}
		return attr & FILE_ATTRIBUTE_DIRECTORY;
	}

	void File::Path::mkdir(const char *dirname) {

		if(!(dirname && *dirname)) {
			throw system_error(EINVAL,system_category(),"Unable to create an empty dirname");
		}

		Win32::Path path{dirname};

		// Try to create the full path first.
		if(!::mkdir(path.c_str())) {
			return;
		} else if(errno == EEXIST) {
			if(File::Path::dir(path.c_str())) {
				return;
			}
			throw system_error(ENOTDIR,system_category(),path);
		}

		// walk the full path and try creating each element
		// Reference: https://github.com/GNOME/glib/blob/main/glib/gfileutils.c

		debug("Creating path '",path.c_str(),"'");
		size_t mark = path.find("\\",1);
		while(mark != string::npos) {
			path[mark] = 0;
			debug("Creating '",path.c_str(),"'");
			if(::mkdir(path.c_str())) {
				if(errno == EEXIST) {
					if(!File::Path::dir(path.c_str())) {
						throw system_error(ENOTDIR,system_category(),path.c_str());
					}
				} else {
					throw system_error(errno,system_category(),path.c_str());
				}
			}

			path[mark] = '\\';
			mark = path.find("\\",mark+1);
		}

		if(::mkdir(path.c_str())) {
			if(errno == EEXIST) {
				if(!File::Path::dir(path.c_str())) {
					throw system_error(ENOTDIR,system_category(),path.c_str());
				}
			} else {
				throw system_error(errno,system_category(),path.c_str());
			}
		}
	}

	void File::Path::mkdir() const {
		mkdir(c_str());
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

	bool File::Path::for_each(const char *pathname, const char *pattern, bool recursive, const std::function<bool (bool, const char *)> &call) {

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

				if(Win32::Path::dir(filename.c_str())) {

					if(recursive) {
						rc = for_each(filename.c_str(), pattern, recursive, call);
					}

					if(rc && PathMatchSpec(de->d_name,pattern)) {
						rc = call(true, filename.c_str());
					}

				} else if(PathMatchSpec(de->d_name,pattern)) {

					rc = call(false, filename.c_str());

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

