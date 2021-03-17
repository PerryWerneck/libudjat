
#include <config.h>
#include <iostream>
#include <udjat/tools/mmap.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <system_error>
#include <sys/mman.h>
#include <cstring>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif // HAVE_UNISTD_H

using namespace std;

namespace Udjat {

	MemoryMappedFile::MemoryMappedFile(const char *path) : fd(open(path,O_RDONLY)),contents(nullptr),length(0) {

		if(fd < 0) {
			cerr << "Can't open " << path << ": " << strerror(errno) << endl;
			throw system_error(errno, system_category());
		}

		struct stat st;
		if(fstat(fd, &st)) {
			cerr << "Can't get size of " << path << ": " << strerror(errno) << endl;
			throw system_error(errno, system_category());
		}

		length = st.st_size;
		this->contents = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);

		if(contents == MAP_FAILED) {

			if(errno == ENODEV) {
				throw runtime_error("The underlying filesystem of the specified file does not support memory mapping.");
			}

			cerr << "Can't map " << path << ": " << strerror(errno) << endl;
			throw system_error(errno, system_category());
		}

#ifdef DEBUG
		cout 	<< "File: " << path << " Length: " << length << " contents: " << this->contents << endl
				<< (const char *) this->contents << endl;
#endif // DEBUG

	}

	MemoryMappedFile::~MemoryMappedFile() {

		if(fd > 0)
			close(fd);

		if(contents)
			munmap(contents,length);

	}

}
