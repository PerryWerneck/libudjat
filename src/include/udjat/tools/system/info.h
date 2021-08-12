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

 namespace Udjat {

	namespace System {

		/// @brief System info (wrapper for sysinfo()).
		struct UDJAT_API Info {

			long uptime;       				///< @brief Seconds since boot.
			unsigned long loads[3];			///< @brief 1, 5, and 15 minute load averages.
			unsigned long long totalram;	///< @brief Total usable main memory size in bytes.
			unsigned long long freeram;		///< @brief Available memory size in bytes.
			unsigned long long sharedram;	///< @brief Amount of shared memory in bytes.
			unsigned long long bufferram;	///< @brief Memory used by buffers in bytes.
			unsigned long long totalswap;	///< @brief Total swap space size in bytes.
			unsigned long long freeswap;	///< @brief Swap space still available in bytes.
			unsigned short procs;			///< @brief Number of current processes.

			Info();

		};

	}

 }
