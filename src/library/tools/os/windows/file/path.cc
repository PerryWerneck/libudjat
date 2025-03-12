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
 
 #include <shlwapi.h>
 #include <dirent.h>
 #include <udjat/win32/exception.h>
 #include <udjat/win32/path.h>
 #include <udjat/tools/file/path.h>
 #include <private/win32.h>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <shlobj.h>
 
 using namespace std;

 namespace Udjat {

	static void expand_folder(std::string &str, const char *key, REFKNOWNFOLDERID id) {

		const char *ptr = Udjat::String::strcasestr(str.c_str(),key);
		if(ptr) {
			try {
				str.replace(ptr - str.c_str(),strlen(key),Win32::KnownFolder{id});
			} catch(const std::exception &e) {
				throw runtime_error(string{key} + ": " + e.what());
			}
		}

	}

	void File::Path::expand(std::string &str) {
		if(!str.empty()) {
			expand_folder(str,"${home}",FOLDERID_Profile);
			expand_folder(str,"${documents}",FOLDERID_Documents);
			expand_folder(str,"${desktop}",FOLDERID_Desktop);
			expand_folder(str,"${sysdata}",FOLDERID_ProgramData);
		}
	}

	File::Path::Path(const File::Path::Type type, const char *str) {

		PWSTR wcp = NULL;
		HRESULT hr;

		const char *suffix = "\\";

		// https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid
		switch(type) {
		case Desktop:
			hr = SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &wcp);
			break;

		case Download:
			hr = SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &wcp);
			break;

		case Templates:
			hr = SHGetKnownFolderPath(FOLDERID_Templates, 0, NULL, &wcp);
			break;

		case PublicShare:
			hr = SHGetKnownFolderPath(FOLDERID_PublicDocuments, 0, NULL, &wcp);
			break;

		case Documents:
			hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &wcp);
			break;

		case Music:
			hr = SHGetKnownFolderPath(FOLDERID_Music, 0, NULL, &wcp);
			break;

		case Pictures:
			hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &wcp);
			break;

		case Videos:
			hr = SHGetKnownFolderPath(FOLDERID_Videos, 0, NULL, &wcp);
			break;

		case SystemData:
			hr = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &wcp);
			break;

		case UserData:
#ifdef FOLDERID_AppDataProgramData
			hr = SHGetKnownFolderPath(FOLDERID_AppDataProgramData, 0, NULL, &wcp);
#else
			hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &wcp);
#endif // FOLDERID_AppDataProgramData
			break;

		default:
			throw runtime_error("Unexpected path type");

		}

		string result;

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

		append(suffix);
		mkdir();

		if(str && *str) {
			append(str);
			append("\\");
			mkdir();
		}
	}

	Win32::Path::Path(const char *pathname) : std::string(pathname) {

		for(char *ptr = (char *) c_str();*ptr;ptr++) {
			if(*ptr == '/') {
				*ptr = '\\';
			}
		}

		if(at(size()-1) == '\\') {
			resize(size()-1);
		}

		char path[MAX_PATH+1];
		memset(path,0,MAX_PATH+1);

		if(!GetFullPathName(c_str(),MAX_PATH,path,NULL)) {
			throw Win32::Exception(c_str());
		}

		assign(path);
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

	bool File::Path::mkdir(const char *dirname, bool required, int UDJAT_UNUSED(mode)) {

		if(!(dirname && *dirname)) {
			throw system_error(EINVAL,system_category(),"Unable to create an empty dirname");
		}

		Win32::Path path{dirname};

		// Try to create the full path first.
		if(!::mkdir(path.c_str())) {
			return true;
		} else if(errno == EEXIST) {
			if(File::Path::dir(path.c_str())) {
				return true;
			} else if(required) {
				throw system_error(ENOTDIR,system_category(),path);
			}
			return false;
		}

		// walk the full path and try creating each element
		// Reference: https://github.com/GNOME/glib/blob/main/glib/gfileutils.c
		size_t mark = path.find("\\",1);
		while(mark != string::npos) {
			path[mark] = 0;
			if(::mkdir(path.c_str())) {
				if(errno == EEXIST) {
					if(!File::Path::dir(path.c_str())) {
						throw system_error(ENOTDIR,system_category(),path.c_str());
					}
				} else if(required) {
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
				return true;

			} else if(required) {
				throw system_error(errno,system_category(),path.c_str());
			} else {
				return false;
			}
		}

		return true;
	}

	void File::Path::mkdir(int mode) const {
		mkdir(c_str(),mode);
	}

	File::Path::Path(int UDJAT_UNUSED(fd)) {
		throw system_error(ENOTSUP,system_category(),"Not available on windows");
	}

	void File::Path::save(const char *filename, const char *contents) {

		File::Temporary tempfile{filename};

		tempfile.write(contents);
		tempfile.save();

	}

	bool File::Path::for_each(const std::function<bool (const File::Path &path, const File::Stat &st)> &call) const {

		Win32::Path path{c_str()};

		if(!dir()) {
			struct stat s;
			if(stat(path.c_str(),&s) != 0) {
				throw system_error(errno,system_category(),this->c_str());
			}
			return call(*this,s);
		}

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

				struct stat s;
				if(stat(filename.c_str(),&s) != 0) {
					throw system_error(errno,system_category(),filename.c_str());
				}

				call(filename,s);

			}


		} catch(...) {

			closedir(dir);
			throw;

		}

		closedir(dir);

		return rc;

	}

	bool File::Path::for_each(const std::function<bool (const File::Path &path)> &call, bool recursive) const {

		if(!dir()) {
			return call(*this);
		}

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

