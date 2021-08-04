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
 #include <udjat/disk/stat.h>
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

	void Disk::Stat::reset(Disk::Stat::Data &data) const {
		data.read = data.write = 0;

		data.saved_read.blocks = read.blocks;
		data.saved_read.time = read.time;

		data.saved_write.blocks = write.blocks;
		data.saved_write.time = write.time;
	}

	void Disk::Stat::compute(Disk::Stat::Data &data) const {

		// Get this cicle values.
		float blocks_read = read.blocks - data.saved_read.blocks;
		float blocks_write = write.blocks - data.saved_write.blocks;
		float time_read = read.time - data.saved_read.time;
		float time_write = write.time - data.saved_write.time;

		// Reset for next cicle.
		reset(data);

		// Compute response.
		float blocksize = (float) this->getBlockSize();

		if(blocks_read > 0 && time_read > 0) {
			data.read = (blocks_read * blocksize) / (time_read/1000.0);
		} else {
			data.read = 0;
		}

		if(blocks_write > 0 && time_write > 0) {
			data.write = (blocks_write * blocksize) / (time_write/1000.0);
		} else {
			data.write = 0;
		}

	}


 }
