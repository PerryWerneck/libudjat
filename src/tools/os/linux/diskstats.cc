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

 #include <udjat/disk/stat.h>
 #include <udjat/tools/file.h>
 #include <iostream>
 #include <linux/fs.h>
 #include <sys/ioctl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

	static const char * next(const char *ptr) {

		while(!isspace(*ptr)) {

			ptr++;
			if(!*ptr) {
				throw system_error(EINVAL, system_category(),"Unexpected format in /proc/diskstats");
			}

		}

		while(isspace(*ptr)) {

			ptr++;
			if(!*ptr) {
				throw system_error(EINVAL, system_category(),"Unexpected format in /proc/diskstats");
			}

		}

		return ptr;

	}

	static void parse(Disk::Stat &st, const char *ptr) {

		// https://www.kernel.org/doc/Documentation/iostats.txt

		ptr = next(ptr);
		st.major = atoi(ptr);

		ptr = next(ptr);
		st.minor = atoi(ptr);

		{
			const char *from = next(ptr);
			ptr = next(from);
			st.name = string{from,((size_t) (ptr-from)) - 1};
		}


		auto sz = sscanf(
			ptr,
			"%lu %lu %lu %u %lu %lu %lu %u %u %u %u %lu %lu %lu %lu",
			&st.read.count,
			&st.read.merged,
			&st.read.blocks,
			&st.read.time,

			&st.write.count,
			&st.write.merged,
			&st.write.blocks,
			&st.write.time,

			&st.io.inprogress,
			&st.io.time,
			&st.io.weighted,

			&st.discards.count,
			&st.discards.merged,
			&st.discards.blocks,
			&st.discards.time
		);

		if(sz != 15) {
			throw system_error(EINVAL, system_category(),"Unexpected format in /proc/diskstats");
		}

	}

	std::list<Disk::Stat> Disk::Stat::get() {

		// https://www.kernel.org/doc/Documentation/iostats.txt
		File::Text proc("/proc/diskstats");

		std::list<Disk::Stat> stats;
		for(auto it = proc.begin(); it != proc.end(); it++) {

			Stat st;
			parse(st,it->c_str());
			stats.push_back(st);

		}

		return stats;

	}

	Disk::Stat::Stat(const char *name) : Stat() {

		if(name && *name) {

			if(!strncasecmp(name,"/dev/",5)) {
				name += 5;
			}

			this->name = name;

			// https://www.kernel.org/doc/Documentation/block/stat.txt
			File::Text proc( (string{"/sys/block/"} + name + "/stat").c_str() );

			auto sz = sscanf(
				next(proc.c_str()),
				"%lu %lu %lu %u %lu %lu %lu %u %u %u %u %lu %lu %lu %lu",
				&read.count,		// read I/Os       requests      number of read I/Os processed
				&read.merged,		// read merges     requests      number of read I/Os merged with in-queue I/O
				&read.blocks,		// read sectors    sectors       number of sectors read
				&read.time,			// read ticks      milliseconds  total wait time for read requests

				&write.count,		// write I/Os      requests      number of write I/Os processed
				&write.merged,		// write merges    requests      number of write I/Os merged with in-queue I/O
				&write.blocks,		// write sectors   sectors       number of sectors written
				&write.time,		// write ticks     milliseconds  total wait time for write requests

				&io.inprogress,		// in_flight       requests      number of I/Os currently in flight
				&io.time,			// io_ticks        milliseconds  total time this block device has been active
				&io.weighted,		// time_in_queue   milliseconds  total wait time for all requests

				&discards.count,	// discard I/Os    requests      number of discard I/Os processed
				&discards.merged,	// discard merges  requests      number of discard I/Os merged with in-queue I/O
				&discards.blocks,	// discard sectors sectors       number of sectors discarded
				&discards.time		// discard ticks   milliseconds  total wait time for discard requests
			);

			if(sz != 15) {
				throw system_error(EINVAL, system_category(),string{"Unexpected format in /sys/block/"} + name + "/stat");
			}

		} else {

			File::Text proc("/proc/diskstats");

			for(auto it = proc.begin(); it != proc.end(); it++) {

				Stat st;
				parse(st,it->c_str());
				if(st.major != 0 && st.minor == 0) {
					*this += st;
				}

			}

		}

	}

	Disk::Stat & Disk::Stat::operator+=(const Disk::Stat &s) {

		major = minor = 0;
		name.clear();

		read.count += s.read.count;
		read.merged += s.read.merged;
		read.blocks += s.read.blocks;
		read.time += s.read.time;

		write.count += s.write.count;
		write.merged += s.write.merged;
		write.blocks += s.write.blocks;
		write.time += s.write.time;

		io.inprogress += s.io.inprogress;
		io.time += s.io.time;
		io.weighted += s.io.weighted;

		discards.count += s.discards.count;
		discards.merged += s.discards.merged;
		discards.blocks += s.discards.blocks;
		discards.time += s.discards.time;

		return *this;
	}

	size_t Disk::Stat::getBlockSize() const {

		// Reference:
		// https://stackoverflow.com/questions/40068904/portable-way-to-determine-sector-size-in-linux

#ifdef BLKSSZGET

		int fd = open((string{"/dev/"} + name).c_str(),O_RDONLY);
		if(fd < 0) {
			throw system_error(errno, system_category(),string{"Cant open '/dev/"} + name + "'");
		}
		size_t blockSize = 0;
		if(ioctl(fd, BLKSSZGET, &blockSize)) {
			int rc = errno;
			close(fd);
			throw system_error(rc, system_category(),string{"Cant get '/dev/"} + name + "' block size");
		}
		close(fd);

		return blockSize;
#else

		return 512;

#endif // BLKSSZGET

	}

 }

