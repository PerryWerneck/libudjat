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
#include <udjat/tools/file/handler.h>

namespace Udjat {

	namespace File {

		/// @brief Temporary file.
		class UDJAT_API Temporary : public File::Handler {
		private:
			/// @brief The reference filename.
			std::string filename;

#ifdef _WIN32
			std::string tempname;
#endif // _WIN32

		protected:

			typedef Temporary super;

		public:

			/// @brief Create unnamed temporary file.
			Temporary();

			/// @brief Create temporary file with same path of another one
			/// @param filename Filename to use as reference.
			Temporary(const char *filename);

			Temporary(const std::string &filename) : Temporary{filename.c_str()} {
			}

			~Temporary();

			/// @brief Get the reference file last modification timestamp.
			/// @return 0 if file is empty or not found.
			time_t mtime() const override;

			/// @brief Create an empty temporary file.
			/// @param len Required file size.
			static std::string create(unsigned long long len);

			/// @brief Create an empty temporary file.
			static std::string create();

			/// @brief Create an empty temporary dir.
			static std::string mkdir();

#ifdef _WIN32

			inline const char * tempfilename() const noexcept {
				return tempname.c_str();
			}

#else

			/// @brief Hardlink tempfile to new filename (Linux only).
			/// @param filename The hard link name.
			void link(const char *filename) const;

#endif // _WIN32

			/// @brief Save tempfile to new filename.
			/// @param filename The file name.
			/// @param replace If true just replace the file, no backup.
			void save(const char *filename, bool replace = false);

			/// @brief Move temporary file to the reference filename.
			/// @param replace If true just replace the file, no backup.
			void save(bool replace = false);

		};

	}

}

