/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/file/handler.h>
 #include <iostream>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/intl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/tools/string.h>
 #include <sstream>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 #ifndef _GNU_SOURCE
	#define _GNU_SOURCE
 #endif // _GNU_SOURCE
 #include <fcntl.h>

 using namespace std;

 namespace Udjat {

	File::Handler::Handler(const char *filename, bool write) 
		: 	Handler{::open(String{filename}.expand().c_str(),O_CREAT|(write ? O_RDWR : O_RDONLY), S_IRUSR | S_IWUSR)} {

		if(fd < 0) {
			throw system_error(errno,system_category(),filename);
		}

		debug(filename,"=",fd);

	}

	File::Handler::~Handler() {
		if(fd > 0) {
			::close(fd);
			fd = -1;
		}
	}

	time_t File::Handler::mtime() const {

		struct stat st;

		if(fstat(fd,&st)) {
			if(errno == ENOENT) {
				return 0;
			}
			throw std::system_error(errno,std::system_category());
		}

		return st.st_size ? st.st_mtime : 0;

	}

#ifdef _WIN32

	void File::Handler::allocate(unsigned long long) {
	}

	void File::Handler::truncate(unsigned long long) {
	}

	size_t File::Handler::write(unsigned long long offset, const void *contents, size_t length) {

		size_t rc = length;

		while(length) {

			if(::lseek(fd, offset, SEEK_SET) < 0) {
				throw system_error(errno,system_category(),"Cant set file offset");
			}

			ssize_t bytes = ::write(fd, contents, length);
			if(bytes < 1) {
				throw system_error(errno,system_category(),"Cant write to file");
			}

			length -= bytes;
			offset += bytes;

			contents = (void *) (((uint8_t *) contents) + bytes);

		}

		return rc;

	}

	size_t File::Handler::read(unsigned long long offset, void *contents, size_t length, bool required) {

		size_t complete = 0;

		do {

			if(::lseek(fd, offset, SEEK_SET) < 0) {
				throw system_error(errno,system_category(),"Cant set file offset");
			}

			ssize_t bytes = ::read(fd,contents,length);
			if(bytes < 0) {
				throw system_error(errno,system_category(),"Cant read from file");
			} else if(bytes == 0) {
				break;
			}

			complete += bytes;

		} while(required && ((size_t) complete) < length);

		return complete;

	}

#else

	void File::Handler::allocate(unsigned long long length) {

		if(!length) {
			return;
		}

		if(fallocate(fd,0,0,length) != 0) {
			throw std::system_error(errno,std::system_category());
		}

	}

	void File::Handler::truncate(unsigned long long length) {

		if(ftruncate(fd,length) != 0) {
			throw std::system_error(errno,std::system_category());
		}

	}

	size_t File::Handler::write(unsigned long long offset, const void *contents, size_t length) {

		size_t rc = length;

		while(length) {

			ssize_t bytes = ::pwrite(fd, contents, length, offset);
			if(bytes < 1) {
				throw system_error(errno,system_category(),"Cant write to file");
			}

			length -= bytes;
			offset += bytes;

			contents = (void *) (((uint8_t *) contents) + bytes);

		}

		return rc;

	}

	void File::Handler::append(void *contents, size_t length) {
		
		off_t to = lseek(fd,0,SEEK_END);
		if(to == (off_t) -1) {
			throw system_error(errno,system_category(),_("Cant set file offset"));
		}

		size_t bsize = block_size();
	
		while(length) {

			ssize_t bytes = (length > bsize) ? bsize : length;

			if(pwrite(fd,contents,bytes,to) < 0) {
				throw system_error(errno,system_category(),_("Cant write to file"));
			}

			to += bytes;
			contents = (void *) (((uint8_t *) contents) + bytes);
			length -= bytes;

		}

	}

	void File::Handler::append(const File::Handler &file, size_t offset) {

		off_t to = lseek(fd,0,SEEK_END);
		if(to == (off_t) -1) {
			throw system_error(errno,system_category(),_("Cant set file offset"));
		}

		off_t from = lseek(file.fd,offset,SEEK_SET);
		if(to == (off_t) -1) {
			throw system_error(errno,system_category(),_("Cant set file offset"));
		}

		size_t bsize = file.block_size();
		char buffer[bsize];
	
		while(ssize_t bytes = pread(file.fd,buffer,bsize,from)) {

			if(bytes < 0) {
				throw system_error(errno,system_category(),_("Cant read from file"));
			}

			if(pwrite(fd,buffer,bytes,to) < 0) {
				throw system_error(errno,system_category(),_("Cant write to file"));
			}

			to += bytes;
			from += bytes;

		}

	}

	size_t File::Handler::read(unsigned long long offset, void *contents, size_t length, bool required) {

		size_t complete = 0;

		do {

			ssize_t bytes = ::pread(fd,contents,length,offset);
			if(bytes < 0) {
				throw system_error(errno,system_category(),_("Cant read from file"));
			} else if(bytes == 0) {
				break;
			}
			complete += bytes;

		} while(required && ((size_t) complete) < length);

		return complete;

	}

#endif // _WIN32


	size_t File::Handler::write(const void *contents, size_t length) {

		size_t rc = length;

		while(length) {

			ssize_t bytes = ::write(fd, contents, length);
			if(bytes < 1) {
				throw system_error(errno,system_category(),_("Cant write to file"));
			}

			length -= bytes;
			contents = (void *) (((uint8_t *) contents) + bytes);

		}

		return rc;

	}

	String File::Handler::read(unsigned long long offset, ssize_t length) {
		
		if(length < 0) {
			length = this->length();
		}

		char buffer[length+1];
		buffer[length] = 0;

		size_t bytes = read(offset,buffer,length,true);
		buffer[bytes] = 0;

		return String{buffer};
	}

	size_t File::Handler::read(void *contents, size_t length, bool required) {

		size_t complete = 0;

		do {

			ssize_t bytes = ::read(fd,contents,length);
			if(bytes < 0) {
				throw system_error(errno,system_category(),_("Cant read from file"));
			} else if(bytes == 0) {
				break;
			}

			contents = (void *) (((uint8_t *) contents) + bytes);
			complete += bytes;
			length -= bytes;

		} while(required && ((size_t) complete) < length);

		return complete;

	}

	unsigned long long File::Handler::length() const {

		struct stat st;
		if(fstat(fd,&st)) {
			throw system_error(errno,system_category(),_("Cant get file length"));
		}

		return st.st_size;
	}

	unsigned long long File::Handler::block_size() const {

#ifdef _WIN32

		return 512ULL;

#else

		struct stat st;
		if(fstat(fd,&st)) {
			throw system_error(errno,system_category(),_("Cant get block size"));
		}

		return st.st_blksize;

#endif // _WIN32
	}

	void File::Handler::save(const std::function<void(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &write) const {

		struct stat st;
		if(fstat(fd,&st)) {
			throw system_error(errno,system_category(),_("Cant get file length"));
		}

		unsigned long long offset = 0;
#ifdef _WIN32
		char buffer[512];
#else
		char buffer[st.st_blksize+1];
#endif // _WIN32

		while(offset < (unsigned long long) st.st_size) {

#ifdef _WIN32
			if(lseek(fd, offset, SEEK_SET) < 0) {
				throw system_error(errno,system_category(),_("Cant set file offset"));
			}
			ssize_t bytes = ::read(fd,buffer,512);
#else
			ssize_t bytes = pread(fd,buffer,st.st_blksize,offset);
#endif // _WIN32
			if(bytes < 0) {
				throw system_error(errno,system_category(),_("Cant read from file"));
			} else if(bytes == 0) {
				throw runtime_error(_("Unexpected EOF reading from file"));
			}

			write(offset,st.st_size,buffer,bytes);

			offset += bytes;

		}

	}

 }

