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
#include <udjat/tools/file.h>
#include <udjat/tools/string.h>

namespace Udjat {

	namespace File {

		/// @brief File Path.
		class UDJAT_API Path : public Udjat::String {
		public:

			enum Type : uint8_t {
				Desktop,
				Download,
				Templates,
				PublicShare,
				Documents,
				Music,
				Pictures,
				Videos,
				SystemData,	///< @brief System data dir ( /usr/share )
				UserData,	///< @brief User data dir ( ~/.local/share )
			};

			Path(const Type type, const char *v = nullptr);

			Path(const char *v = nullptr);

			Path(const char *v, size_t s);

			Path(const char *dir, const char *name);
			
			Path(const std::string &v) : Path(v.c_str()) {
			}

			static void expand(std::string &str);

			File::Path & expand();

			Path(int fd);

			/// @brief Create directory.
			/// @param dirname The directory name.
			/// @param required if true launch exception on failure.
			/// @param mode The directory mode
			/// @return true if the directory was created.
			static bool mkdir(const char *dirname, bool required = true, int mode = 0755);

			/// @brief Check if path is a directory.
			/// @param pathname the pathname to check;
			/// @return true if pathname is a directory.
			static bool dir(const char *pathname);

			/// @brief Check if path is a regular file.
			/// @param pathname the pathname to check;
			/// @return true if pathname is a regular file.
			static bool regular(const char *pathname);

			const char * name() const noexcept;

			inline bool dir() const {
				return dir(c_str());
			}

			inline bool regular() const {
				return regular(c_str());
			}

			/// @brief Check if file match wildcard.
			static bool match(const char *pathname, const char *pattern) noexcept;

			/// @brief Check if file match wildcard.
			inline bool match(const char *pattern) const noexcept {
				return match(c_str(),pattern);
			}

			/// @brief Create directory.
			void mkdir(int mode = 0755) const;

			/// @brief Find file in the path, replace value if found.
			/// @return true if 'name' was found and the object value was updated.
			bool find(const char *name, bool recursive = false);

			/// @brief Convert path in the canonicalized absolute pathname.
			File::Path & realpath();

			/// @brief Test if the file is valid.
			operator bool() const noexcept;

			/// @brief Navigate on all directory files until lambda returns 'true'
			/// @param call Lambda for file test.
			/// @return false if all 'call' actions returned false.
			/// @retval true call() has returned 'true', scan was finished.
			/// @retval false All files were scanned, call never returned 'true'.
			bool for_each(const std::function<bool (const File::Path &path, const Stat &st)> &call) const;

			/// @brief Navigate on all directory files until lambda returns 'true'
			/// @param call Lambda for file test.
			/// @return false if all 'call' actions returned false.
			/// @retval true call() has returned 'true', scan was finished.
			/// @retval false All files were scanned, call never returned 'true'.
			bool for_each(const std::function<bool (const File::Path &path)> &call, bool recursive = false) const;

			/// @brief Navigate on directory files until lambda returns 'true'
			/// @param pattern File filter pattern.
			/// @param call Lambda for file test.
			/// @return false if all 'call' actions returned false.
			/// @retval true call() has returned 'true', scan was finished.
			/// @retval false All files were scanned, call never returned 'true'.
			bool for_each(const char *pattern, const std::function<bool (const File::Path &path)> &call, bool recursive = false) const;

			/// @brief Recursive remove of files.
			void remove(bool force = false);

			/// @brief Load file.
			String load() const;

			/// @brief Save file.
			static void save(const char *filename, const char *contents);

			/// @brief Save file to FD.
			static void save(int fd, const char *contents);

			/// @brief Save file with custom writer.
			void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer) const;

			/// @brief Replace file without backup
			static void replace(const char *filename, const char *contents);

			/// @brief Save file.
			inline void save(const char *contents) const {
				save(c_str(),contents);
			}

		};

	}

	namespace XML {

		File::Path UDJAT_API PathFactory(const char *p);

	}

}
