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

 #ifndef _GNU_SOURCE
	#define _GNU_SOURCE             /* See feature_test_macros(7) */
 #endif // _GNU_SOURCE

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

	File::Temporary::Temporary() : fd{open("/tmp",O_TMPFILE|O_RDWR, S_IRUSR | S_IWUSR)} {

		if(fd < 0) {
			throw system_error(errno,system_category(),"Can't create transient temporary file");
		}

	}

	File::Temporary::Temporary(const char *name) : filename{name} {

		char path[PATH_MAX];
		strncpy(path,name,PATH_MAX);

		fd = open(dirname(path),O_TMPFILE|O_RDWR, S_IRUSR | S_IWUSR);
		if(fd > 0) {
			return;
		}

		if(errno != ENOTSUP) {
			throw system_error(errno,system_category(),Logger::Message("Can't create '{}'",filename));
		}

		strncpy(path,name,PATH_MAX); // Dirname change buffer, copy it again.
		File::Path directory{dirname(path)};
		if(!directory.dir()) {
			throw system_error(ENOTDIR,system_category(),directory);
		}

		directory += '/';

		unsigned long seq = (unsigned long) time(0);

		for(size_t retry = 0; retry < 1000; retry++) {

			string filename{directory.c_str()};
			filename += std::to_string(seq--);

			debug("Trying ",filename);

			fd = open(filename.c_str(),O_CREAT|O_EXCL|O_RDWR, S_IRUSR | S_IWUSR);
			if(fd > 0) {
				return;
			}

			if(errno != EEXIST) {
				throw system_error(errno,system_category(),Logger::Message("Can't create '{}'",filename));
			}

		}

		throw runtime_error(Logger::Message("Can't create temporary file on '{}'",directory.c_str()));

	}

	File::Temporary::~Temporary() {
		::close(fd);
	}

	std::string File::Temporary::mkdir() {

		string basename{"/tmp/"};
		basename += Application::Name();

		if(::mkdir(basename.c_str(),0777) && errno != EEXIST) {
			throw system_error(errno,system_category(),string{"Can't create '"} + basename + "'");
		}

		basename += "/";
		basename += TimeStamp().to_string("%Y%m%d%H%M%s");

		for(size_t f = 0; f < 1000; f++) {

			string filename = basename + "." + std::to_string(rand()) + ".tmp";

			if(::mkdir(filename.c_str(), 0700) == 0) {
				return filename;
			}

			if(errno != EEXIST) {
				throw system_error(errno,system_category(),string{"Can't create '"} + filename + "'");
			}

		}

		throw runtime_error(string{"Too many files in '/tmp/'" PACKAGE_NAME});

	}

	std::string File::Temporary::create() {
		return create(0);
	}

	std::string File::Temporary::create(unsigned long long len) {

		string basename{"/tmp/"};
		basename += Application::Name();

		if(::mkdir(basename.c_str(),0777) && errno != EEXIST) {
			throw system_error(errno,system_category(),string{"Can't create '"} + basename + "'");
		}

		basename += "/";
		basename += TimeStamp().to_string("%Y%m%d%H%M%s");

		for(size_t f = 0; f < 1000; f++) {

			string filename = basename + "." + std::to_string(rand()) + ".tmp";
			int fd = open(filename.c_str(),O_CREAT|O_EXCL|O_RDWR,0600);
			if(fd > 0) {
				if(len && fallocate(fd, 0, 0, len) != 0) {
					int err = errno;
					::close(fd);
					throw system_error(err,system_category(),string{"Can't allocate '"} + filename + "'");
				}
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
			char realname[PATH_MAX];
			ssize_t namelen = readlink(tempfile,realname,PATH_MAX);

			if(namelen > 0 && namelen < PATH_MAX) {

				realname[namelen] = 0;

				if(rename(realname, filename) != 0) {
					Udjat::File::save(fd,filename);
				}

			} else {

				Udjat::File::save(fd,filename);

			}

		}

		chmod(filename,st.st_mode);
		chown(filename,st.st_uid,st.st_gid);

	}

	void File::Temporary::link(const char *filename) const {
		File::move(fd,filename);
	}

	void File::Temporary::save(bool replace) {

		if(filename.empty()) {
			throw system_error(EINVAL,system_category(),"No target filename");
		}

		this->save(filename.c_str(),replace);
	}

	ssize_t File::Temporary::write(const void *contents, size_t length) {

		ssize_t rc = length;

		while(length) {

			ssize_t bytes = ::write(fd, contents, length);
			if(bytes < 1) {
				throw system_error(errno,system_category(),"Cant write to temporary file");
			}

			length -= bytes;
			contents = (void *) (((uint8_t *) contents) + bytes);

		}

		return rc;

	}

	ssize_t File::Temporary::read(void *contents, size_t length, bool required) {

		ssize_t complete = 0;

		do {

			ssize_t bytes = ::read(fd,contents,length);
			if(bytes < 0) {
				throw system_error(errno,system_category(),"Cant read from temporary file");
			} else if(bytes == 0) {
				break;
			}
			complete += bytes;

		} while(required && ((size_t) complete) < length);

		return complete;

	}

 }
