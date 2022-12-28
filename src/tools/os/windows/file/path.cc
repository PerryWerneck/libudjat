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

		if(!(pathname && *pathname)) {
			return false;
		}

		DWORD attr = GetFileAttributes(pathname);
		if(attr == INVALID_FILE_ATTRIBUTES) {
			throw Win32::Exception(pathname);
		}
		return attr & FILE_ATTRIBUTE_DIRECTORY;
	}

	bool File::Path::regular(const char *pathname) {

		if(!(pathname && *pathname)) {
			return false;
		}

		DWORD attr = GetFileAttributes(pathname);
		if(attr == INVALID_FILE_ATTRIBUTES) {
			throw Win32::Exception(pathname);
		}
		return attr & FILE_ATTRIBUTE_NORMAL;
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

	bool File::Path::for_each(const std::function<bool (const File::Path &path)> &call, bool recursive) {

		Win32::Path path{c_str()};

		debug("Scanning '",path.c_str(),"'");

		DIR *dir = opendir(path.c_str());

		if(!dir) {
			throw system_error(errno,system_category(),path);
		}

		bool rc = false;

		try {

			struct dirent *de;
			while(!rc && (de = readdir(dir)) != NULL) {

				if(de->d_name[0] == '.') {
					continue;
				}

				File::Path filename{path};
				filename += "\\";
				filename += de->d_name;

				if(filename.dir()) {
					if(recursive) {
						rc = filename.for_each(call,recursive);
					}
				} else {
					rc = call(filename);
				}

			}


		} catch(...) {

			closedir(dir);
			throw;

		}

		closedir(dir);

		return rc;
	}

	bool File::Path::match(const char *pathname, const char *pattern) noexcept {
		return PathMatchSpec(pathname,pattern);
	}

	void File::Path::remove(bool force) {

		Win32::Path path{c_str()};

		DIR *dir = opendir(path.c_str());

		if(!dir) {
			throw system_error(errno,system_category(),path);
		}

		try {

			struct dirent *de;
			while((de = readdir(dir)) != NULL) {

				File::Path filename{path};
				filename += "\\";
				filename += de->d_name;

				if(filename.dir()) {

					filename.remove(force);
					if(RemoveDirectory(filename.c_str()) == 0) {
						auto rc = GetLastError();
						if(rc == ERROR_DIR_NOT_EMPTY) {
							MoveFileEx(filename.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
							clog << "win32\tDirectory '" << filename << "' will be removed on next reboot" << endl;
						} else {
							throw Win32::Exception(filename.c_str(),rc);
						}
					}

				} else {

					if(DeleteFile(filename.c_str()) == 0) {
						auto rc = GetLastError();
						if(rc == ERROR_ACCESS_DENIED) {
							MoveFileEx(filename.c_str(),NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
							clog << "win32\tFile '" << filename << "' will be removed on next reboot" << endl;
						} else {
							throw Win32::Exception(filename.c_str(),rc);
						}
					}

				}

			}

		} catch(...) {

			closedir(dir);
			throw;

		}
		closedir(dir);

	}

}

