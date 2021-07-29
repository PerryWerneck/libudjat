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

		File::Text proc("/proc/diskstats");

		for(auto it = proc.begin(); it != proc.end(); it++) {

			parse(*this,it->c_str());
			if(!strcasecmp(this->name.c_str(),name)) {
				return;
			}

		}

		throw system_error(ENOENT, system_category(), (string{"Can't find diskstats for '"} + name + "'"));

	}

 }

