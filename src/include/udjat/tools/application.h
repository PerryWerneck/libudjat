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
 #include <udjat/tools/application.h>
 #include <string>

 namespace Udjat {

	namespace Application {

		/// @brief The application name.
		class UDJAT_API Name : public std::string {
		public:
			/// @brief Get application name.
			/// @param with_path when false get the complete application name with path.
			Name(bool with_path = false);

			static const Name & getInstance();

		};

#ifdef _WIN32
		class UDJAT_API Path : public std::string {
		public:
			Path();
		};

		class UDJAT_API LogDir : public std::string {
		public:
			LogDir();
		};
#endif // _WIN32

		class UDJAT_API DataDir : public std::string {
		public:
			DataDir();
			DataDir(const char *subdir);
		};

		/// @brief File from the application datadir.
		class UDJAT_API DataFile : public std::string {
		public:
			DataFile(const char *name);
		};

		class UDJAT_API LibDir : public std::string {
		public:
			LibDir();

			/// @brief True if the path exists.
			operator bool() const noexcept;

			/// @brief Create path to application subdir below the system's library path.
			LibDir(const char *subdir);

			/// @brief Set application name.
			void reset(const char *application_name, const char *subdir);

		};

		class UDJAT_API SysConfigDir : public std::string {
		public:
			SysConfigDir();

			/// @brief Create path to application subdir below the system's configuration path.
			SysConfigDir(const char *subdir);
		};


	}

 }

