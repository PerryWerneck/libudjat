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
 #include <udjat/tools/disk/stat.h>
 #include <udjat/tools/xml.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	static const Disk::Unit units[] = {
		{          1.0,  "B",  "B/s" },
		{       1024.0, "KB", "KB/s" },
		{    1048576.0, "MB", "MB/s" },
		{ 1073741824.0, "GB", "GB/s" }
	};

	const Disk::Unit * Disk::Unit::get(const pugi::xml_node &node, const char *attr, const char *def) {
		return get(Attribute(node,attr).as_string(def));
	}

	const Disk::Unit * Disk::Unit::get(const char *name) {

		for(size_t ix = 0; ix < ((sizeof(units)/sizeof(units[0]))); ix++) {
			if(name[0] == units[ix].id[0]) {
				return &units[ix];
			}
		}

		throw runtime_error(string{"Invalid unit '"} + name + "'");

	}

	Disk::Stat::Data & Disk::Stat::reset(Disk::Stat::Data &data) const {
		data.read = data.write = 0;

		float blocksize = getBlockSize();

		data.saved.read.bytes = ((float) read.blocks) * blocksize;
		data.saved.read.time = read.time;

		data.saved.write.bytes = ((float) write.blocks) * blocksize;
		data.saved.write.time = write.time;

		return data;
	}

	Disk::Stat::Data & Disk::Stat::compute(Disk::Stat::Data &data) const {

		// Get blocksize
		float blocksize = getBlockSize();

		// Get this cicle values.
		float bytes_read = (read.blocks * blocksize) - data.saved.read.bytes;
		float bytes_write = (write.blocks * blocksize) - data.saved.write.bytes;
		float time_read = read.time - data.saved.read.time;
		float time_write = write.time - data.saved.write.time;

		// Reset for next cicle.
		{
			data.read = data.write = 0;

			data.saved.read.bytes = ((float) read.blocks) * blocksize;
			data.saved.read.time = read.time;

			data.saved.write.bytes = ((float) write.blocks) * blocksize;
			data.saved.write.time = write.time;
		}

		if(bytes_read > 0 && time_read > 0) {
			data.read = bytes_read / (time_read/1000.0);
		} else {
			data.read = 0;
		}

		if(bytes_write > 0 && time_write > 0) {
			data.write = bytes_write / (time_write/1000.0);
		} else {
			data.write = 0;
		}

		return data;
	}

	Disk::Stat::Data::Data(const Disk::Stat &stat) : Data() {

		float blocksize = (float) stat.getBlockSize();

		saved.read.bytes = ((float) stat.read.blocks) * blocksize;
		saved.read.time = stat.read.time;

		saved.write.bytes = ((float) stat.write.blocks) * blocksize;
		saved.write.time = stat.write.time;

	}

	Disk::Stat::Data & Disk::Stat::Data::operator+=(const Disk::Stat &stat) {

		Disk::Stat::Data data(stat);

		read += data.read;
		write += data.write;

		saved.read.bytes += data.saved.read.bytes;
		saved.read.time += data.saved.read.time;

		saved.write.bytes += data.saved.write.bytes;
		saved.write.time += data.saved.write.time;

		return *this;

	}

	Disk::Stat::Data & Disk::Stat::Data::operator+=(const Disk::Stat::Data &data) {

		read += data.read;
		write += data.write;

		saved.read.bytes += data.saved.read.bytes;
		saved.read.time += data.saved.read.time;

		saved.write.bytes += data.saved.write.bytes;
		saved.write.time += data.saved.write.time;

		return *this;

	}

	Disk::Stat::Data & Disk::Stat::Data::update(const Disk::Stat::Data &data) {

		// Get disk read info.
		{
			float bytes_read = data.saved.read.bytes - saved.read.bytes;
			float time_read = data.saved.read.time - saved.read.time;

			if(bytes_read > 0 && time_read > 0) {
				read = bytes_read / (time_read/1000.0);
			} else {
				read = 0;
			}
		}

		// Get disk write info.
		{
			float bytes_write = data.saved.write.bytes - saved.write.bytes;
			float time_write = data.saved.write.time - saved.write.time;

			if(bytes_write > 0 && time_write > 0) {
				write = bytes_write / (time_write/1000.0);
			} else {
				write = 0;
			}
		}

		// Store for next cicle.
		saved.read.bytes = data.saved.read.bytes;
		saved.read.time = data.saved.read.time;

		saved.write.bytes = data.saved.write.bytes;
		saved.write.time = data.saved.write.time;

		return *this;
	}

 }
