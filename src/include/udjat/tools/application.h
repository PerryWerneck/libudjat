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
 #include <udjat/tools/xml.h>
 #include <list>
 #include <string>
 #include <ostream>
 #include <memory>

 namespace Udjat {

	/// @brief Base class for applications.
	class UDJAT_API Application {
	public:
		Application();

		enum Type : uint8_t {
			Generic,		///< @brief Generic application.
			AdminTool,		///< @brief Administrative tool.
		};

		/// @brief Setup locale.
		/// @param gettext_package The gettext package name.
		static void UDJAT_API set_gettext_package(const char *gettext_package);

		/// @brief Initialize application; setup locale.
		/// @param definitions	The xml file for application definitions.
		static int UDJAT_API init(int argc, char **argv, const char *definitions = nullptr);

		/// @brief Load XML application definitions.
		/// @param pathname Path to a single xml file or a folder with xml files.
		/// @param force Do a reconfiguration even if the file hasn't change.
		/// @return Seconds for reconfiguation.
		static time_t UDJAT_API setup(const char *pathname, bool force = false);

		/// @brief Finalize application.
		static int UDJAT_API finalize();

		/// @brief Write to the 'information' stream.
		static std::ostream & info();

		/// @brief Write to the 'warning' stream.
		static std::ostream & warning();

		/// @brief Write to the 'error' stream.
		static std::ostream & error();

		/// @brief Write to the 'trace' stream.
		static std::ostream & trace();

		/// @brief Install application shortcut in the autostart folder.
		/// @param type The application type.
		/// @param id The application id.
		/// @param name The shortcut name.
		/// @param comment Shortcut comment.
		/// @param Arguments for application.
		void autostart(const Type type, const char *id, const char *name = "", const char *comment = "", const char *arguments = "");

		inline void autostart(const char *id, const char *name = "", const char *comment = "", const char *arguments = "") {
			autostart(Generic,id,name,comment,arguments);
		}

		/// @brief Install application Shortcut.
		/// @param type The application type.
		/// @param id The application id.
		/// @param name The shortcut name.
		/// @param comment Shortcut comment.
		/// @param Arguments for application.
		void shortcut(const Type type, const char *id, const char *name = "", const char *comment = "", const char *arguments = "");

		inline void shortcut(const char *id, const char *name = "", const char *comment = "", const char *arguments = "") {
			shortcut(Generic,id,name,comment,arguments);
		}

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
		class UDJAT_API Path : public File::Path {
		public:
			Path(const char *subdir = nullptr);
		};

		class UDJAT_API InstallLocation : public File::Path {
		public:
			InstallLocation();

			operator bool() const;

		};

#else

		/// @brief Send signal to all instances of this application.
		/// @param signo Signal to send (0 to just count instances).
		/// @return Count of detected instances.
		static size_t signal(int signum = 0);

#endif // _WIN32

		class UDJAT_API LogDir : public File::Path {
		public:
			LogDir(const char *subdir = nullptr);
			static LogDir & getInstance();
		};

		/// @brief Application data dir.
		class UDJAT_API DataDir : public File::Path {
		public:
			DataDir(const char *subdir = nullptr);
		};

		/// @brief System datadir
		class UDJAT_API SystemDataDir : public File::Path {
		public:
			SystemDataDir(const char *subdir = nullptr);
		};

		/// @brief User datadir
		class UDJAT_API UserDataDir : public File::Path {
		public:
			UserDataDir(const char *subdir = nullptr);
		};

		/// @brief File from the application or system datadir.
		class UDJAT_API DataFile : public File::Path {
		public:

			/// @brief Create a full path to datafile based on XML definition.
			/// @param node XML node for file definition.
			/// @param attrname Attribute for filename
			/// @param system When true use the systemdatadir for file path if necessary.
			DataFile(const XML::Node &node, const char *attrname = "path", bool system = true);

			/// @brief Create a full path to datafile based on name and XML definition.
			/// @param type Datafile type (subdirectory).
			/// @param node XML node for file definition.
			/// @param attrname Attribute for filename
			/// @param system When true use the systemdatadir for file path if necessary.
			DataFile(const char *type, const XML::Node &node, const char *attrname = "path", bool system = true);

			/// @brief Create a full path to datafile.
			/// @param name	The file name.
			/// @param system When true use the systemdatadir for file path if necessary.
			DataFile(const char *name, bool system = false);

			/// @brief Is the file available?
			/// @return true if the file is available.
			bool available() const noexcept;

			inline operator bool() const noexcept {
				return available();
			}

		};

		class UDJAT_API LibDir : public File::Path {
		public:
			LibDir(const char *subdir = nullptr);

			/// @brief True if the path exists.
			operator bool() const noexcept;

			/// @brief Set application name.
			void reset(const char *application_name, const char *subdir);

		};

		class UDJAT_API SysConfigDir : public File::Path {
		public:
			SysConfigDir(const char *subdir = nullptr);
		};

		class UDJAT_API CacheDir : public File::Path {
		public:
			CacheDir(const char *subdir = nullptr);

			/// @brief Build cachename for file
			/// @param name File or URL name.
			/// @return Full path for cached file.
			std::string build_filename(const char *name);

		};

	};

 }

