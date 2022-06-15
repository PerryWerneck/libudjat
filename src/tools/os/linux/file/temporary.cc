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
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/param.h>
 #include <fcntl.h>
 #include <libgen.h>
 #include <udjat/tools/logger.h>
 #include <cstring>
 #include <cstdio>
 #include <system_error>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/application.h>

 using namespace std;

 namespace Udjat {

	File::Temporary::Temporary(const char *name) : filename{name} {

		char path[PATH_MAX];
		strncpy(path,name,PATH_MAX);

		fd = open(dirname(path),O_TMPFILE|O_RDWR, S_IRUSR | S_IWUSR);
		if(fd < 0) {
			throw system_error(errno,system_category(),Logger::Message("Can't create '{}'",filename));
		}

	}

	File::Temporary::~Temporary() {
		::close(fd);
	}

	std::string File::Temporary::create() {

		string basename{"/tmp/"};
		basename += Application::Name();

		if(mkdir(basename.c_str(),0777) && errno != EEXIST) {
			throw system_error(errno,system_category(),string{"Can't create '"} + basename + "'");
		}

		basename += "/";
		basename += TimeStamp().to_string("%Y%m%d%H%M%s");

		for(size_t f = 0; f < 1000; f++) {

			string filename = basename + "." + std::to_string(rand()) + ".tmp";
			int fd = open(filename.c_str(),O_CREAT|O_EXCL,0600);
			if(fd > 0) {
				::close(fd);
				return filename;
			}

			if(errno != EEXIST) {
				throw system_error(errno,system_category(),string{"Can't create '"} + filename + "'");
			}

		}

		throw runtime_error(string{"Too many files in '/tmp/'" PACKAGE_NAME});

	}

	void File::Temporary::save(const char *filename, bool replace) {

		//
		// First get target file attributes
		//
		struct stat st;
		if(stat(filename, &st) == -1) {

			// Error getting filename, assume defaults if not found.

			if(errno != ENOENT) {
				throw system_error(errno,system_category(),"Error getting file information");
			}

			memset(&st,0,sizeof(st));
			st.st_mode = 0644;

		} else if(replace) {

			// Replace, remove current file.
			unlink(filename);

		} else {

			// Not replace, create '.bak' file.
			char bakfile[PATH_MAX];
			strncpy(bakfile,filename,PATH_MAX);
			char *ptr = strrchr(bakfile,'.');
			if(ptr) {
				*ptr = 0;
			}
			strncat(bakfile,".bak",PATH_MAX);

			unlink(bakfile);
			if(rename(filename, bakfile) != 0) {
				throw system_error(errno,system_category(),"Cant create backup");
			}

		}

		//
		// Try to hardlink file
		//
		char tempfile[PATH_MAX];
		snprintf(tempfile, PATH_MAX,  "/proc/self/fd/%d", fd);
		if(linkat(AT_FDCWD, tempfile, AT_FDCWD, filename, AT_SYMLINK_FOLLOW) != 0) {

			// Unable to create hardlink, try to save file.
			Udjat::File::save(fd,filename);

		}

		chmod(filename,st.st_mode);
		chown(filename,st.st_uid,st.st_gid);

	}

	void File::Temporary::link(const char *filename) const {

		char tempfile[PATH_MAX];
		snprintf(tempfile, PATH_MAX,  "/proc/self/fd/%d", fd);

		struct stat st;
		if(stat(filename, &st) == -1) {
			if(errno != ENOENT) {
				throw system_error(errno,system_category(),"Error getting file information");
			}
			memset(&st,0,sizeof(st));
			st.st_mode = 0644;
		}

		if(linkat(AT_FDCWD, tempfile, AT_FDCWD, filename, AT_SYMLINK_FOLLOW) != 0) {
			//
			// Cant link file
			//

			if(errno != EEXIST) {
				throw system_error(errno,system_category(),"Error saving file");
			}

			char bakfile[PATH_MAX];
			strncpy(bakfile,filename,PATH_MAX);
			char *ptr = strrchr(bakfile,'.');
			if(ptr) {
				*ptr = 0;
			}
			strncat(bakfile,".bak",PATH_MAX);

			unlink(bakfile);
			if(rename(filename, bakfile) != 0) {
				throw system_error(errno,system_category(),"Cant create backup");
			}

			if(linkat(AT_FDCWD, tempfile, AT_FDCWD, filename, AT_SYMLINK_FOLLOW) != 0) {
				throw system_error(errno,system_category(),"Error saving file");
			}

		}

		chmod(filename,st.st_mode);
		chown(filename,st.st_uid,st.st_gid);

	}

	void File::Temporary::save(bool replace) {

		if(filename.empty()) {
			throw system_error(EINVAL,system_category(),"No target filename");
		}

		this->save(filename.c_str(),replace);
	}

	File::Temporary & File::Temporary::write(const void *contents, size_t length) {

		while(length) {

			ssize_t bytes = ::write(fd, contents, length);
			if(bytes < 1) {
				throw system_error(errno,system_category(),"Cant write to temporary file");
			}

			length -= bytes;
			contents = (void *) (((uint8_t *) contents) + bytes);

		}
		return *this;

	}


 }
