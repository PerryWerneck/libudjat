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
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <libgen.h>

 namespace Udjat {

	File::Local::Local(const char *path)  {

		int fd = open(path,O_RDONLY);
		if(fd < 0) {
			throw system_error(errno, system_category(), (string{"Can't open '"} + path + "'"));
		}

		try {

			struct stat st;
			if(fstat(fd, &st)) {
				throw system_error(errno, system_category(), "Cant get file size");
			}

			this->length = st.st_size;

			load(fd);

		} catch(...) {

			::close(fd);
			throw;

		}

	}

	File::Local::Local(int fd, ssize_t length) {

		if(length < 0) {

			struct stat st;
			if(fstat(fd, &st)) {
				throw system_error(errno, system_category(), "Can't get file size");
			}

			this->length = st.st_size;

		} else {

			this->length = length;

		}

		load(fd);
	}

	void File::Local::load(int fd) {

		if(length) {

			// Have the file length, use mmap.
			this->mapped = true;
			this->contents = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);

			if(contents == MAP_FAILED) {

				if(errno == ENODEV) {
					throw runtime_error("The underlying filesystem of the specified file does not support memory mapping.");
				}

				throw system_error(errno, system_category(), "Cant map file contents");
			}

		} else {

			// No file length, uses the 'normal way'.
			mapped = false;
			ssize_t in;
			size_t szBuffer = 4096;
			length = 0;
			contents = malloc(szBuffer);

			char * ptr = (char *) contents;
			while( (in = read(fd,ptr,szBuffer - length)) != 0) {

				if(in < 0) {
					throw system_error(errno, system_category(), "Cant read file");
				}

				if(in > 0) {
					ptr += in;
					length += in;
					if(length >= (szBuffer - 20)) {
						szBuffer += 4096;
						contents = realloc(contents,szBuffer);
						ptr = (char *) contents+length;
					}
				}

			}

			contents = (char *) realloc(contents,length+1);
			((char *) contents)[length] = 0;

		}

	}

	File::Local::Local(const File::Agent &agent) : Local(agent.c_str()) {
	}

	File::Local::~Local() {
		if(mapped) {
			munmap(contents,length);
		} else {
			free(contents);
		}
	}

	void File::Local::forEach(std::function<void (const string &line)> call) {
		forEach(c_str(),call);
	}

	void File::Local::set(const char *contents) {
		if(mapped) {
			munmap(this->contents,length);
			mapped = false;
		} else {
			free(this->contents);
		}

		this->contents = strdup(contents);
		this->length = strlen((const char *) this->contents);

	}

	void File::Local::forEach(const char *from, std::function<void (const string &line)> call) {
		while(from) {
			const char *to = strchr(from,'\n');
			if(to) {
				call(string(from,to-from));
				from = to+1;
			} else {
				call(string(from));
				break;
			}
		}

	}

	void File::Local::save(const char *filename) {

		// Not implemented.
		if(!filename) {
			throw system_error(ENOTSUP,system_category(),"Can't retrieve filename");
		}

		// Get file information.
		struct stat st;

		memset(&st,0,sizeof(st));
		if(stat(filename, &st) == -1) {
			if(errno != ENOENT) {
				throw system_error(errno, system_category(), string{"Can't get output stats file when saving '"} + filename + "'");
			}
		}

		if(mapped) {
			// Unmap file; saving it will change contents.
			munmap(this->contents,length);
			mapped = false;
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

 }
