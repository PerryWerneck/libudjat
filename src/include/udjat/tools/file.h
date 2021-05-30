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
		class Controller;

		/// @brief File Path.
		class UDJAT_API Path {
		private:
			std::string value;

		public:
			Path(const char *v) : value{v} {
			}

			Path(int fd);

			/// @brief Save file.
			static void save(const char *filename, const char *contents);

			/// @brief Save file.
			/// @param filename File name.
			inline void save(const char *contents) const {
				save(value.c_str(),contents);
			}

		};

		/// @brief Generic text file object (Don't use for large files).
		class UDJAT_API Text : public Path {
		private:
			char *contents = nullptr;
			bool mapped = false;
			size_t length = 0;

			void unload();
			void load(int fd, ssize_t length = -1);

		public:

			class UDJAT_API Iterator {
			private:
				friend class Text;

				/// @brief Pointer to text file contents.
				const char *text;

				/// @brief Text length;
				size_t length;

				/// @brief Current position in the text.
				size_t	offset;

				/// @brief Value of the current row;
				std::string value;

				Iterator & set(size_t offset);

			public:
				using iterator_category = std::forward_iterator_tag;
				using value_type        = std::string;

				const std::string * operator*() const {
					return &value;
				}

				std::string * operator->() {
					return &value;
				}

				// Prefix increment
				Iterator& operator++();

				// Postfix increment
				Iterator operator++(int);

				friend bool operator== (const Iterator& a, const Iterator& b) {
					return a.offset == b.offset && a.text == b.text;
				};

				friend bool operator!= (const Iterator& a, const Iterator& b) {
					return a.offset != b.offset || a.text != b.text;
				};

			};

			Text(int fd, ssize_t length = -1);
			Text(const char *filename);
			~Text();

			inline const char * c_str() const noexcept {
				return this->contents;
			}

			const Iterator begin() const noexcept;
			const Iterator end() const noexcept;

			void set(const char *contents);

			static void forEach(const char *text, std::function<void (const std::string &line)> call);

			inline void forEach(std::function<void (const std::string &line)> call) const {
				forEach(contents,call);
			}

			void save() const;

		};

		/// @brief File watcher.
		class UDJAT_API Watcher {
		private:

			static std::mutex guard;
			friend class File::Controller;

			int wd = -1;
			Quark name;

			/// @brief Time of last modification.
			time_t mtime = 0;

			/// @brief True if all the children were updated.
			bool updated = false;

			Watcher(const Quark &name);
			~Watcher();

			/// @brief Text file watcher.
			struct File {
				void *id;
				std::function<void (const Udjat::File::Text &file)> callback;

				File(void *i, std::function<void (const Udjat::File::Text &)> c) : id(i), callback(c) {
				}

			};

			/// @brief Text file watchers.
			std::list<File> files;

			void onEvent(const uint32_t event) noexcept;
			void onChanged() noexcept;

		public:

			/// @brief Update all children (if necessary).
			/// @return true if the update was done.
			bool update(bool force = false);

			void remove(void *id);

			inline const char * c_str() const {
				return name.c_str();
			}

			void push_back(void *id, std::function<void (const Udjat::File::Text &)> callback);

		};

		/// @brief Insert a file/folder watcher.
		UDJAT_API Watcher * watch(void *id, const Quark &name, std::function<void (const Udjat::File::Text &)> callback);

		/// @brief Insert a file/folder watcher.
		UDJAT_API Watcher * watch(void *id, const char *name, std::function<void (const Udjat::File::Text &)> callback);

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
			virtual void set(const File::Text &file);

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
