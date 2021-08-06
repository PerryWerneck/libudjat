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
				TOTAL
			};

			unsigned long user;		///< @brief Normal processes.
			unsigned long nice;		///< @brief Niced processes.
			unsigned long system;	///< @brief Kernel mode.
			unsigned long idle;		///< @brief twiddling thumbs.
			unsigned long iowait;	///< @brief Waiting for I/O.
			unsigned long irq;		///< @brief CPU used when servicing interrupts.
			unsigned long softirq;	///< @brief Soft IRQS.

			/// @brief Create object with data from /proc/stat.
			Stat();

			/// @brief The typenames.
			static const char *typenames[];

			/// @brief Get type from typename.
			static Type getIndex(const char *name);

			unsigned long operator[](const Type ix) const;
			unsigned long operator[](const char *name) const;

			/// @brief Sum all value.
			/// @brief The sum of all fields (including idle).
			unsigned long total() const noexcept;

			Stat & operator-=(const Stat &stat);

		};

	}

 }
