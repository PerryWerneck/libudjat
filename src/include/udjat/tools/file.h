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
#include <udjat/tools/quark.h>
#include <udjat/tools/file/handler.h>
#include <udjat/tools/file/temporary.h>
#include <list>
#include <algorithm>
#include <mutex>
#include <string>
#include <cstring>
#include <cstdint>
#include <sys/stat.h>
#include <udjat/tools/string.h>

namespace Udjat {

	namespace File {

		using Stat = struct stat;

		class Agent;
		class Controller;

		/// @brief Copy file
		UDJAT_API void copy(int from, const char *to);

		inline void save(int from, const char *to) {
			copy(from,to);
		}

		/// @brief Copy file with custom writer.
		UDJAT_API void copy(const char *from, const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer);

		/// @brief Copy file
		UDJAT_API void copy(const char *from, const char *to, bool replace = true);

		UDJAT_API void copy(const char *from, const char *to, const std::function<bool(double current, double total)> &progress, bool replace = true);

		/// @brief Set file modification time.
		/// @return 0 if ok, non zero if not (sets errno).
		UDJAT_API int mtime(const char *filename, time_t time);

		/// @brief Get file modification time.
		UDJAT_API time_t mtime(const char *filename);

		/// @brief Check file modification time.
		/// @param filename The file name to check.
		/// @param max_age The maximum age in seconds.
		/// @return true if file is outdated, false otherwise.
		/// @note If file does not exist, it is considered outdated.
		UDJAT_API bool outdated(const char *filename, time_t max_age);

#ifdef _WIN32

		UDJAT_API int move(const char *from, const char *to, bool replace = false);

#else
		/// @brief Move file, create 'bak'
		/// @param from The origin file name.
		/// @param to The target file name.
		/// @param no_backup if false '.bak' file will be created with old contents.
		UDJAT_API int move(const char *from, const char *to, bool no_backup = false);

		/// @brief Move file, create 'bak'
		/// @param The handle of an open origin file.
		/// @param to The target file name.
		/// @param no_backup if false '.bak' file will be created with old contents.
		UDJAT_API int move(int fd, const char *to, bool no_backup = false);

#endif // _WIN32


		/// @brief Save to temporary file.
		/// @param contents String with file contents.
		/// @return Temporary file name.
		UDJAT_API std::string save(const char *contents);


	}

}
