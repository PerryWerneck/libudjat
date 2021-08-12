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

 #include <udjat/defs.h>
 #include <udjat/tools/system/info.h>
 #include <sys/sysinfo.h>
 #include <errno.h>
 #include <system_error>

 using namespace std;

 namespace Udjat {

	namespace System {

		Info::Info() {

			struct sysinfo info;

			if(sysinfo(&info)) {
				throw system_error(errno,system_category(),"Can't get system information");
			}

			this->procs = info.procs;

			unsigned long long mem_unit = (unsigned long long) info.mem_unit;

			this->totalram = ((unsigned long long) info.totalram) * mem_unit;
			this->freeram = ((unsigned long long) info.freeram) * mem_unit;
			this->sharedram = ((unsigned long long) info.sharedram) * mem_unit;
			this->bufferram = ((unsigned long long) info.bufferram) * mem_unit;
			this->totalswap = ((unsigned long long) info.totalswap) * mem_unit;
			this->freeswap = ((unsigned long long) info.freeswap) * mem_unit;

			for(size_t ix = 0; ix < 3; ix++) {
				this->loads[ix] = info.loads[ix];
			}

		}

	}

 }
