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
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <libgen.h>
 #include <dirent.h>
 #include <fnmatch.h>
 #include <iostream>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	File::Path::Path(const File::Path::Type, const char *) {
		throw system_error(ENOTSUP,system_category(),"Invalid operation for this value");
	}

	File::Path::Path(int UDJAT_UNUSED(fd)) {
	}

	void File::Path::expand(std::string &) {
	}

	void File::Path::save(int fd, const char *contents) {

		size_t length = strlen(contents);
		while(length > 0) {

			auto wrote = write(fd,contents,length);
			if(wrote < 0) {
				throw system_error(errno, system_category());
			} if(!wrote) {
				throw runtime_error("Unexpected EOF writing file");
			}

			length -= wrote;
			contents += wrote;

		}
	}

	bool File::Path::dir(const char *pathname) {

		if(!(pathname && *pathname)) {
			return false;
		}

		struct stat s;
		if(stat(pathname,&s) != 0) {
			if(errno == ENOENT) {
				return false;
			}
			throw system_error(errno,system_category(),pathname);
		}
		return (s.st_mode & S_IFDIR) != 0;
	}

	bool File::Path::regular(const char *pathname) {

		if(!(pathname && *pathname)) {
			return false;
		}

		struct stat s;
		if(stat(pathname,&s) != 0) {
			if(errno == ENOENT) {
				return false;
			}
			throw system_error(errno,system_category(),pathname);
		}
		return (s.st_mode & S_IFREG) != 0;
	}

	bool File::Path::mkdir(const char *dirname, bool required, int mode) {

		if(!(dirname && *dirname)) {
			throw system_error(EINVAL,system_category(),"Unable to create an empty dirname");
		}

		// Try to create the full path first.
		if(!::mkdir(dirname,mode)) {
			return true;
		} else if(errno == EEXIST) {
			if(File::Path::dir(dirname)) {
				return true;
			}
			throw system_error(ENOTDIR,system_category(),dirname);
		}

		// walk the full path and try creating each element
		// Reference: https://github.com/GNOME/glib/blob/main/glib/gfileutils.c

		string path{dirname};
		if(path[path.size()-1] == '/') {
			path.resize(path.size()-1);
		}

		size_t mark = path.find("/",1);
		while(mark != string::npos) {
			path[mark] = 0;
			if(::mkdir(path.c_str(),mode)) {
				if(errno == EEXIST) {
					if(!File::Path::dir(path.c_str())) {
						throw system_error(ENOTDIR,system_category(),path.c_str());
					}
				} else if(required) {
					throw system_error(errno,system_category(),path.c_str());
				}
			}

			path[mark] = '/';
			mark = path.find("/",mark+1);
		}

		if(::mkdir(path.c_str(),mode)) {
			if(errno == EEXIST) {
				if(!File::Path::dir(path.c_str())) {
					throw system_error(ENOTDIR,system_category(),path.c_str());
				}
			} else if(required) {
				throw system_error(errno,system_category(),path.c_str());
			} else {
				return false;
			}
		}
		return true;
	}

	void File::Path::mkdir(int mode) const {
		mkdir(c_str(),true,mode);
	}

	void File::Path::replace(const char *filename, const char *contents) {

#ifdef _WIN32
		int out = open(filename,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0644);
#else
		int out = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0644);
#endif // _WIN32

		try {

			save(out,contents);

		} catch(...) {
			::close(out);
			throw;
		}

		::close(out);

	}

	void File::Path::save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const {
		File::copy(c_str(),writer);
	}

	void File::Path::save(const char *filename, const char *contents) {

		// Get file information.
		struct stat st;

		memset(&st,0,sizeof(st));
		if(stat(filename, &st) == -1) {
			if(errno != ENOENT) {
				throw system_error(errno, system_category(), string{"Can't get output stats file when saving '"} + filename + "'");
			}

			// Setup defaults.
			st.st_uid = getuid();
			st.st_gid = getgid();
			st.st_mode = 0644;

		}

		// Save file
		string tempfile;
		int fd;

		{

			static const char *mask = "/FXXXXXX";
			size_t buflen = strlen(filename) + strlen(mask) + 1;
			char *buffer = new char[buflen+1];

			strncpy(buffer,filename,buflen);
			strncat(dirname(buffer),mask,buflen);

			fd = mkostemp(buffer,O_WRONLY|O_APPEND);

			int e = errno;

			tempfile = buffer;
			delete[] buffer;

			if(fd < 0) {
				throw system_error(e, system_category(), string{"Can't create output file when saving '"} + filename + "'");
			}

		}

		try {

			save(fd,contents);

		} catch(...) {

			::close(fd);
			::remove(tempfile.c_str());
			throw;
		}

		::close(fd);

		// Temp file saved, create backup and rename the new file.
		try {

			if(::access(filename,F_OK) == 0) {
				string backup = (string{filename} + "~");
				if(::remove(backup.c_str()) == -1 && errno != ENOENT) {
					cerr << "Error '" << strerror(errno) << "' removing '" << backup.c_str() << "'" << endl;
				}
				if(link(filename,backup.c_str()) == -1) {
					cerr << "Error '" << strerror(errno) << "' creating backup of '" << filename <<  "'" << endl;
				}
			}

			::remove(filename);
			if(link(tempfile.c_str(),filename) == -1) {
				throw system_error(errno, system_category(), string{"Error saving '"} + filename + "'");
			}

			// Set permissions.
			chmod(filename,st.st_mode);

			// Set owner and group.
			if(chown(filename, st.st_uid, st.st_gid) == -1) {
				throw system_error(errno, system_category(), string{"Error setting owner & group on '"} + filename + "'");
			}

			::remove(tempfile.c_str());

		} catch(...) {
			::remove(tempfile.c_str());
			throw;
		}

	}

	bool File::Path::match(const char *pathname, const char *pattern) noexcept {
		return fnmatch(pattern,pathname,FNM_CASEFOLD) == 0;
	}

	void File::Path::remove(bool UDJAT_UNUSED(force)) {

		char path[PATH_MAX+1];
		if(!::realpath(c_str(),path)) {
			throw system_error(errno,system_category(),*this);
		}

		if(!dir()) {
			debug("Removing '",path,"'");
			if(unlink(path)) {
				throw system_error(errno,system_category(),path);
			}
			return;
		}

		// It's a folder, navigate.
		DIR *dir = opendir(path);

		if(!dir) {
			throw system_error(errno,system_category(),path);
		}

		bool rc = false;

		try {

			struct dirent *de;
			while(!rc && (de = readdir(dir)) != NULL) {

				if(!de->d_name || de->d_name[0] == '.') {
					continue;
				}

				File::Path filename{path};
				filename += "/";
				filename += de->d_name;
				filename.remove(force);

			}

		} catch(...) {

			closedir(dir);
			throw;

		}

		closedir(dir);

		debug("Removing '",path,"'");
		if(rmdir(path)) {
			throw system_error(errno,system_category(),path);
		}

	}

	File::Path & File::Path::realpath() {
		char path[PATH_MAX+1];
		if(!::realpath(c_str(),path)) {
			throw system_error(errno,system_category(),c_str());
		}
		assign(path);
		return *this;
	}

	bool File::Path::for_each(const std::function<bool (const File::Path &path)> &call, bool recursive) const {

		if(!dir()) {
			return call(*this);
		}

		char path[PATH_MAX+1];
		if(!::realpath(c_str(),path)) {
			throw system_error(errno,system_category(),c_str());
		}

		debug(path);

		// It's a folder, navigate.
		DIR *dir = opendir(path);

		if(!dir) {
			if(errno == ENOTDIR) {
				return false;
			}
			throw system_error(errno,system_category(),*this);
		}

		bool rc = false;

		try {

			struct dirent *de;
			while(!rc && (de = readdir(dir)) != NULL) {

				if(!de->d_name || de->d_name[0] == '.') {
					continue;
				}

				File::Path filename{path};
				filename += "/";
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


}

