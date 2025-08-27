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
 #include <udjat/module/abstract.h>
 #include <udjat/tools/quark.h>

 #include <sys/stat.h>
 #include <udjat/tools/file/text.h>
 #include <fcntl.h>
 #include <libgen.h>

 #include <list>

 using namespace std;

 namespace Udjat {

	File::Text::Text(int fd, ssize_t length) : Path(fd) {
		load(fd,length);
	}

	File::Text::Text(const char *filename) : Path(filename) {

		int fd = open(filename,O_RDONLY|O_BINARY);
		if(fd < 0) {
			throw system_error(errno, system_category(), "Can't open file");
		}

		try {

			load(fd);

		} catch(const exception &e) {

			::close(fd);
			throw;
		}

		::close(fd);
	}

	File::Text::~Text() {
		unload();
	}

	void File::Text::unload() {

		free(contents);
		contents = nullptr;
		length = 0;

	}

	void File::Text::save() const {
		File::Path::save(contents);
	}

	void File::Text::save(const char *name) {
		File::Path::save(name,contents);
	}

	File::Text & File::Text::set(const char *contents) {

		unload();
		this->contents = strdup(contents);
		this->length = strlen(this->contents);

		return *this;

	}

	void File::Text::load(int fd, ssize_t length) {

		if(fd < 0) {
			throw system_error(errno, system_category(), "Can't load file");
		}

		if(length < 0) {

			struct stat st;
			if(fstat(fd, &st)) {
				throw system_error(errno, system_category(), "Cant get file size");
			}

			length = st.st_size;

		}

		unload();

		// Try to load the file.
		mapped = false;
		ssize_t in;
		ssize_t szBuffer = 4096;
		length = 0;
		contents = (char *) malloc(szBuffer);

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
					contents = (char *) realloc(contents,szBuffer);
					ptr = contents+length;
				}
			}

		}

		contents = (char *) realloc(contents,length+1);
		((char *) contents)[length] = 0;
		this->length = length;

	}

	void File::Text::for_each(const char *from, std::function<void (const string &line)> call) {
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

	File::Text::Iterator & File::Text::Iterator::set(size_t offset) {

		this->offset = offset;
		if(this->offset >= this->length) {
			this->value = "";
			return *this;
		}

		const char *from = (this->text + this->offset);
		const char *to = strchr(from,'\n');

		if(to) {
			value = string(from,to-from);
		} else {
			value = string(from);
		}

		return *this;

	}

	File::Text::Iterator & File::Text::Iterator::operator++() {

		const char * from = strchr((text+offset),'\n');

		if(from) {
			from++;
			return set(from-text);
		}

		return set(length);
	}

	File::Text::Iterator File::Text::Iterator::operator++(int) {
		Iterator tmp = *this;
		++(*this);
		return tmp;
	}

	const File::Text::Iterator File::Text::begin() const noexcept {

		Iterator it;
		it.text		= contents;
		it.length	= length;
		it.set(0);

		return it;
	}

	const File::Text::Iterator File::Text::end() const noexcept {

		Iterator it;
		it.text		= contents;
		it.length	= length;
		it.set(length);

		return it;
	}


 }
