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
			&st.read.sectors,
			&st.read.time,

			&st.write.count,
			&st.write.merged,
			&st.write.sectors,
			&st.write.time,

			&st.io.inprogress,
			&st.io.time,
			&st.io.weighted,

			&st.discards.count,
			&st.discards.merged,
			&st.discards.sectors,
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

		// https://www.kernel.org/doc/Documentation/block/stat.txt

		File::Text proc( (string{"/sys/block/"} + name + "/stat").c_str() );

		auto sz = sscanf(
			next(proc.c_str()),
			"%lu %lu %lu %u %lu %lu %lu %u %u %u %u %lu %lu %lu %lu",
			&read.count,		// read I/Os       requests      number of read I/Os processed
			&read.merged,		// read merges     requests      number of read I/Os merged with in-queue I/O
			&read.sectors,		// read sectors    sectors       number of sectors read
			&read.time,			// read ticks      milliseconds  total wait time for read requests

			&write.count,		// write I/Os      requests      number of write I/Os processed
			&write.merged,		// write merges    requests      number of write I/Os merged with in-queue I/O
			&write.sectors,		// write sectors   sectors       number of sectors written
			&write.time,		// write ticks     milliseconds  total wait time for write requests

			&io.inprogress,		// in_flight       requests      number of I/Os currently in flight
			&io.time,			// io_ticks        milliseconds  total time this block device has been active
			&io.weighted,		// time_in_queue   milliseconds  total wait time for all requests

			&discards.count,	// discard I/Os    requests      number of discard I/Os processed
			&discards.merged,	// discard merges  requests      number of discard I/Os merged with in-queue I/O
			&discards.sectors,	// discard sectors sectors       number of sectors discarded
			&discards.time		// discard ticks   milliseconds  total wait time for discard requests
		);

		if(sz != 15) {
			throw system_error(EINVAL, system_category(),string{"Unexpected format in /sys/block/"} + name + "/stat");
		}

	}

 }

