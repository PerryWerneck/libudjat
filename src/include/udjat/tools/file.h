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
#include <list>
#include <algorithm>
#include <mutex>
#include <string>

namespace Udjat {

	namespace File {

		class Agent;

		/// @brief Generic text file object (Don't use for large files).
		class UDJAT_API Local {
		private:
			void * contents = nullptr;	///< @brief File contents.
			bool mapped = false;		///< @brief Is the file mmapped?
			size_t length = 0;			///< @brief File length.

			void load(int fd);

		public:
			Local(int fd, ssize_t length = -1);
			Local(const char *filename);
			Local(const File::Agent &agent);
			~Local();

			inline size_t size() const noexcept {
				return this->length;
			}

			inline const char * c_str() const noexcept {
				return (const char *) contents;
			}

			/// @brief Set file contents.
			void set(const char *contents);

			/// @brief Save file.
			/// @param filename File name.
			void save(const char *filename = nullptr);

			static void forEach(const char *text, std::function<void (const std::string &line)> call);
			void forEach(std::function<void (const std::string &line)> call);

		};

		/// @brief File watcher.
		class UDJAT_API Watcher {
		private:

			static std::mutex guard;

			int wd = -1;
			Quark name;

			/// @brief Time of last modification.
			time_t mtime = 0;

			/// @brief True if all the children were updated.
			bool updated = false;

			class Controller;
			friend class Controller;

			Watcher(const Quark &name);
			~Watcher();

			/// @brief Text file watcher.
			struct File {
				void *id;
				std::function<void (const Udjat::File::Local &file)> callback;

				File(void *i, std::function<void (const Udjat::File::Local &)> c) : id(i), callback(c) {
				}

			};

			/// @brief Text file watchers.
			std::list<File> files;

			void onEvent(const uint32_t event) noexcept;
			void onChanged() noexcept;

		public:
			static Watcher * insert(void *id, const Quark &name, std::function<void (const Udjat::File::Local &)> callback);
			static Watcher * insert(void *id, const char *name, std::function<void (const Udjat::File::Local &)> callback);

			/// @brief Update all children (if necessary).
			/// @return true if the update was done.
			bool update(bool force = false);

			void remove(void *id);

			inline const char * c_str() const {
				return name.c_str();
			}

		};

		/// @brief Directory contents.
		class UDJAT_API List : public std::list<std::string> {
		public:
			List(const char *pattern);
			~List();
			void forEach(std::function<void (const char *filename)> call);
		};

		/// @brief Text file agent.
		///
		/// Monitor a local file and call 'load' method when it changes.
		///
		class UDJAT_API Agent {
		private:

			/// @brief The file watcher
			Watcher * watcher = nullptr;

		protected:

			/// @brief Called wihen the file changes.
			virtual void set(const File::Local &file);

			/// @brief Called when the file changes.
			/// @param The file contents.
			virtual void set(const char *contents);

			/// @brief Reload file contents (if necessary).
			inline bool update(bool force = false) {
				return watcher->update(force);
			}

		public:
			Agent(const char *name);
			Agent(const Quark &name);
			Agent(const pugi::xml_node &node);
			Agent(const pugi::xml_node &node, const char *attribute);
			Agent(const pugi::xml_attribute &attribute);

			inline const char * c_str() const {
				return watcher->c_str();
			}

			inline const char * getPath() const {
				return watcher->c_str();
			}

			virtual ~Agent();

		};

	}

}
