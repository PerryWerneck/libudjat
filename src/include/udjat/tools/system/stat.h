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

		/// @brief Disk stats from /proc/stat.
		struct UDJAT_API Stat {

			enum Type : unsigned short {
				USER,
				NICE,
				SYSTEM,
				IDLE,
				IOWAIT,
				IRQ,
				SOFTIRQ,
				STEAL,
				GUEST,
				GUEST_NICE,
				TOTAL	// allways after the internal ones.

			};

			unsigned long user = 0;			//< @brief normal processes executing in user mode.
			unsigned long nice= 0;			//< @brief niced processes executing in user mode.
			unsigned long system = 0;		//< @brief processes executing in kernel mode.
			unsigned long idle = 0;			//< @brief twiddling thumbs.
			unsigned long iowait = 0;		//< @brief waiting for I/O to complete.
			unsigned long irq = 0;			//< @brief servicing interrupts.
			unsigned long softirq = 0;		//< @brief servicing softirqs.
			unsigned long steal = 0;		//< @brief ticks spent executing other virtual hosts (in virtual environments like Xen).
			unsigned long guest = 0;
			unsigned long guest_nice = 0;

			/// @brief Create object with data from /proc/stat.
			Stat();

			/// @brief The typenames.
			static const char *typenames[];

			/// @brief Get type from typename.
			static Type getIndex(const char *name);

			unsigned long operator[](const Type ix) const;
			unsigned long operator[](const char *name) const;

			/// @brief Get total number of ticks.
			unsigned long getUsage() const noexcept;

			/// @brief Get 'active' tickes (usage - idle).
			unsigned long getRunning() const noexcept;

			inline unsigned long getIdle() const noexcept {
				return idle;
			}

			/// @brief Sum all value.
			/// @brief The sum of all fields (including idle).
			unsigned long total() const noexcept;

			Stat & operator-=(const Stat &stat);

		};

	}

 }
