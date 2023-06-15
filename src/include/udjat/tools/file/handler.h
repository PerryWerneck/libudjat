/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
#include <cstddef>
#include <string>
#include <functional>
#include <cstring>

namespace Udjat {

	namespace File {

		class UDJAT_API Handler {
		protected:
			int fd = -1;

			typedef File::Handler super;

		public:
			constexpr Handler() = default;

			constexpr Handler(int f) : fd{f} {
			}

			Handler(const char *filename, bool write = false);

			virtual ~Handler();

			/// @brief Write data to file at offset.
			/// @param contents Data to write.
			/// @param length Data length.
			/// @return Number of bytes written (allways 'length')
			size_t write(unsigned long long offset, const void *contents, size_t length);

			/// @brief Write data to file.
			/// @param contents Data to write.
			/// @param length Data length.
			/// @return Number of bytes written (allways 'length')
			size_t write(const void *contents, size_t length);

			/// @brief Read data from file.
			/// @param contents The buffer for file contents.
			/// @param length The length of the buffeer.
			/// @param required when true read 'length' bytes.
			/// @return Number of bytes read (0 on eof);
			size_t read(void *contents, size_t length, bool required = false);

			/// @brief Read data from file at offset.
			/// @param contents The buffer for file contents.
			/// @param length The length of the buffeer.
			/// @param required when true read 'length' bytes.
			/// @return Number of bytes read (0 on eof);
			size_t read(unsigned long long offset, void *contents, size_t length, bool required = false);

			inline size_t write(const std::string &str) {
				return write(str.c_str(),str.size());
			}

			inline size_t write(const char *str) {
				return write(str,strlen(str));
			}

			template<typename T>
			inline size_t write(const T &value) {
				return write((const void *) &value, sizeof(value));
			}

			template<typename T>
			inline size_t read(T &value) {
				return read((void *) &value, sizeof(value));
			}

			/// @brief Allocate disk space for file, trigger exception when failed.
			/// @param length Required space for this file.
			void allocate(unsigned long long length);

			/// @brief Truncate file to a specified length
			void truncate(unsigned long long length = 0LL);

			/// @brief Save file using custom writer.
			void save(const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &write) const;

		};

	}

}


namespace std {

	template<typename T>
	inline Udjat::File::Handler & operator<< (Udjat::File::Handler &file, const T &value) {
			file.write(value);
			return file;
	}

	template<typename T>
	inline Udjat::File::Handler & operator>> (Udjat::File::Handler &file, T &value) {
			file.read(value);
			return file;
	}

}

