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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/disk/stat.h>
 #include <string>
 #include <list>

 namespace Udjat {

	namespace Disk {

		/// @brief Disk stats from /proc/diskstats.
		struct UDJAT_API Stat {

			unsigned short major = 0;		///< @brief The major number of the disk.
			unsigned short minor = 0;		///< @brief The minor number of the disk.

			std::string name;				///< @brief Device name.

			unsigned long rdcount = 0;		///< @brief The total number of reads completed successfully.
			unsigned long rdmerged = 0;		///< @brief The number of reads merged.
			unsigned long rdsectors = 0;	///< @brief The total number of sectors read successfully.
			unsigned int  rdtime = 0;		///< @brief The total number of milliseconds spent by all reads.

			unsigned long wrcount = 0;		///< @brief The total number of writes completed successfully.
			unsigned long wrmerged = 0;		///< @brief The number of writes merged.
			unsigned long wrsectors = 0;	///< @brief The total number of sectors written successfully.
			unsigned int  wrtime = 0;		///< @brief The total number of milliseconds spent by all writes.

			unsigned int  inprogress = 0;	///< @brief The number of I/Os currently in progress.
			unsigned int  iotime = 0;		///< @brief The time spent doing I/Os.
			unsigned int  ioweighted = 0;	///< @brief The weighted # of milliseconds spent doing I/Os.

			unsigned long dscount = 0;		///< @brief The total number of discards completed successfully.
			unsigned long dsmerged = 0;		///< @brief The number of discards merged.
			unsigned long dssectors = 0;	///< @brief The total number of sectors discarded successfully.
			unsigned long dstime = 0;		///< @brief The number of milliseconds spent discarding.

			/// @brief Create an empty device.
			Stat() {
			}

			/// @brief Get from device name.
			Stat(const char *name);

			/// @brief Load /proc/diskstats.
			static void load(std::list<Stat> &stats);

		};

	}

 }
