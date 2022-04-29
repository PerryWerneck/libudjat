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

 namespace Udjat {

	File::Path::Path(int UDJAT_UNUSED(fd)) {
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

			size_t length = strlen((const char *) contents);
			const char * ptr = (const char *) contents;
			while(length > 0) {

				auto wrote = write(fd,ptr,length);
				if(wrote < 0) {
					throw system_error(errno, system_category(), string{"Error writing to temp when saving '"} + filename + "'");
				} if(!wrote) {
					throw system_error(errno, system_category(), string{"Unexpected '0' bytes return code when saving '"} + filename + "'");
				}

				length -= wrote;
				ptr += wrote;

			}

		} catch(...) {

			::close(fd);
			remove(tempfile.c_str());
			throw;
		}

		::close(fd);

		// Temp file saved, create backup and rename the new file.
		try {

			if(::access(filename,F_OK) == 0) {
				string backup = (string{filename} + "~");
				if(remove(backup.c_str()) == -1 && errno != ENOENT) {
					cerr << "Error '" << strerror(errno) << "' removing '" << backup.c_str() << "'" << endl;
				}
				if(link(filename,backup.c_str()) == -1) {
					cerr << "Error '" << strerror(errno) << "' creating backup of '" << filename <<  "'" << endl;
				}
			}

			remove(filename);
			if(link(tempfile.c_str(),filename) == -1) {
				throw system_error(errno, system_category(), string{"Error saving '"} + filename + "'");
			}

			// Set permissions.
			chmod(filename,st.st_mode);

			// Set owner and group.
			if(chown(filename, st.st_uid, st.st_gid) == -1) {
				throw system_error(errno, system_category(), string{"Error setting owner & group on '"} + filename + "'");
			}

			remove(tempfile.c_str());

		} catch(...) {
			remove(tempfile.c_str());
			throw;
		}

	}

	bool File::Path::for_each(const char *path, bool recursive, std::function<bool (const char *)> call) {
		return for_each(path,"*",recursive,call);
	}

	bool File::Path::for_each(const char *path, const char *pattern, bool recursive, std::function<bool (const char *)> call) {

		DIR *dir;
		size_t szPath = strlen(path);

		if(path[szPath-1] == '/') {
			szPath--;
			dir = opendir(string(path,szPath).c_str());
		} else {
			dir = opendir(path);
		}

		if(!dir) {
			throw system_error(ENOENT,system_category(),path);
		}

		bool rc = true;

		try {

			struct dirent *de;
			while(rc && (de = readdir(dir)) != NULL) {

				if(!de->d_name || de->d_name[0] == '.') {
					continue;
				}

				string filename(path,szPath);
				filename += "/";
				filename += de->d_name;

				if(recursive && de->d_type == DT_DIR) {

					rc = for_each(filename.c_str(), pattern, recursive, call);

				} else if(fnmatch(pattern,de->d_name,FNM_CASEFOLD) == 0) {

#ifdef DEBUG
					cout << "found '" << filename << "'" << endl;
#endif // DEBUG

					rc = call(filename.c_str());

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

