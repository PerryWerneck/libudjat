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

 /**
  * @brief Declare the file watcher.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>

 namespace Udjat {

	namespace File {

		/// @brief File watcher, launch events when a file/directory changes.
		class UDJAT_API Watcher {
		public:

			/// @brief File watcher event codes.
			enum Event : uint8_t {
				Modified,	///< @brief File was modified.
				Created,	///< @brief File was created.
				Deleted,	///< @brief File was deleted.
				MovedFrom,	///< @brief Generated for the directory containing the old filename when a file is renamed.
				MovedTo		///< @brief Generated for the directory containing the new filename when a file is renamed.
			};

			/// @brief Build a watcher from path.
			/// @param path The file or directory to watch.
			Watcher(const char *pathname);

			/// @brief Build a watcher from XML definition.
			Watcher(const XML::Node &node, const char *attrname = "path");

			~Watcher();

			/// @brief Start file watching.
			Watcher & watch();

			/// @brief Stop file watching.
			Watcher & unwatch();

		protected:

			/// @brief The file/directory path.
			const char *pathname;

			/// @brief Launch file event.
			virtual void updated(const Event event, const char *filename);

		private:
			class Controller;
			friend class Controller;

		};

	}

 }
