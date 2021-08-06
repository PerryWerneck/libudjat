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

 #include <iostream>
 #include <sstream>
 #include <fstream>
 #include <iomanip>
 #include <cstring>
 #include <udjat/tools/system/stat.h>

 using namespace std;

 namespace Udjat {

	const char * System::Stat::typenames[] = { "user", "nice", "system", "idle", "iowait", "irq", "softirq", "total", nullptr };

	System::Stat::Stat() {

		ifstream in("/proc/stat", ifstream::in);
		in.ignore(3);

		in	>> user
			>> nice
			>> system
			>> idle
			>> iowait
			>> irq
			>> softirq;

	}

	System::Stat::Type System::Stat::getIndex(const char *name) {

		for(unsigned short ix = 0; typenames[ix]; ix++) {
			if(!strcasecmp(name,typenames[ix])) {
				return (System::Stat::Type) ix;
			}
		}

		if(!strcasecmp(name,"total")) {
			return TOTAL;
		}

		throw runtime_error(string{"Unexpected property name '"} + name + "'");
	}

	unsigned long System::Stat::operator[](const System::Stat::Type ix) const {

		switch(ix) {
		case USER:
			return user;

		case NICE:
			return nice;

		case SYSTEM:
			return system;

		case IDLE:
			return idle;

		case IOWAIT:
			return iowait;

		case IRQ:
			return irq;

		case SOFTIRQ:
			return softirq;

		case TOTAL:
			return user+nice+system+idle+iowait+irq+softirq;

		}

		throw system_error(EINVAL,system_category(),"Invalid index");

	}

	unsigned long System::Stat::total() const noexcept {
		return user+nice+system+idle+iowait+irq+softirq;
	}

	unsigned long System::Stat::operator[](const char *name) const {
		return (*this)[getIndex(name)];
	}

	System::Stat & System::Stat::operator-=(const System::Stat &stat) {

		user -= stat.user;
		nice -= stat.nice;
		system -= stat.system;
		idle -= stat.idle;
		iowait -= stat.iowait;
		irq -= stat.irq;
		softirq -= stat.softirq;

		return *this;
	}


 }



