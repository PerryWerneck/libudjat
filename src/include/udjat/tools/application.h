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
 #include <udjat/tools/file.h>
 #include <string>
 #include <ostream>

 namespace Udjat {

	/// @brief Base class for applications.
	class UDJAT_API Application {
	public:

		/// @brief Setup locale.
		/// @param gettext_package The gettext package name.
		static void UDJAT_API set_gettext_package(const char *gettext_package);

		/// @brief Initialize application; setup locale.
		/// @return true if the application was initialized.
		static bool UDJAT_API init();

		/// @brief Initialize application; setup locale.
		/// @param definitions	The xml file for application definitions.
		static int UDJAT_API init(int argc, char **argv, const char *definitions = nullptr);

		/// @brief Finalize application.
		static int UDJAT_API finalize();

		/// @brief Write to the 'information' stream.
		static std::ostream & info();

		/// @brief Write to the 'warning' stream.
		static std::ostream & warning();

		/// @brief Write to the 'error' stream.
		static std::ostream & error();

		/// @brief The application name.
		class UDJAT_API Name : public std::string {
		public:
			/// @brief Get application name.
			/// @param with_path when false get the complete application name with path.
			Name(bool with_path = false);

			static const Name & getInstance();

		};

		inline const Name & name() const {
			return Name::getInstance();
		}

#ifdef _WIN32
		class UDJAT_API Path : public std::string {
		public:
			Path();
		};

		class UDJAT_API LogDir : public std::string {
		public:
			LogDir();

		};

		class UDJAT_API InstallLocation : public std::string {
		public:
			InstallLocation();

			operator bool() const;

		};

#endif // _WIN32

		/// @brief Application data dir.
		class UDJAT_API DataDir : public File::Path {
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

		class UDJAT_API CacheDir : public std::string {
		public:
			CacheDir();
			CacheDir(const char *subdir);


		};

	};

 }

