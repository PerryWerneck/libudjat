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
#include <cstring>
#include <sys/stat.h>

namespace Udjat {

	namespace File {

		using Stat = struct stat;

		class Agent;
		class Controller;

		/// @brief Copy file
		void save(int fd, const char *filename);

		/// @brief Copy file
		void copy(const char *from, const char *to);

		/// @brief Save to temporary file.
		/// @param contents String with file contents.
		/// @return Temporary file name.
		std::string save(const char *contents);

		/// @brief File Path.
		class UDJAT_API Path : public std::string {
		public:

			Path() : std::string() {
			}

			Path(const char *v) : std::string(v) {
			}

			Path(const std::string &v) : std::string(v) {
			}

			Path(int fd);

			/// @brief Create directory.
			static void mkdir(const char *dirname);

			static bool dir(const char *pathname);

			/// @brief Create directory.
			void mkdir() const;

			/// @brief Find file in the path, replace value if found.
			/// @return true if 'name' was found and the object value was updated.
			bool find(const char *name, bool recursive = false);

			/// @brief Test if the file is valid.
			operator bool() const noexcept;

			static bool for_each(const char *path, const char *pattern, bool recursive, std::function<bool (const char *)> call);

			/// @brief Navigate on all directory files.
			/// @return false if 'call' has returned false;
			static bool for_each(const char *path, const std::function<bool (const char *name, const Stat &stat)> &call);

			/// @brief Navigate on all directory files and directories.
			/// @return false if 'call' has returned false;
			static bool for_each(const char *pathname, const char *pattern, bool recursive, const std::function<bool (bool isdir, const char *path)> &call);

			/// @brief Execute 'call' on every file on the path, until it returns 'false'.
			/// @return false if 'call' has returned false;
			inline bool for_each(const char *pattern, bool recursive, std::function<bool (const char *filename)> call) const {
				return for_each(c_str(),pattern,recursive,call);
			}

			/// @brief Execute 'call' on every file on the path, until it returns 'false'.
			/// @return false if 'call' has returned false;
			inline bool for_each(bool recursive, std::function<bool (const char *filename)> call) const {
				return for_each(c_str(),"*",recursive,call);
			}

			/// @brief Execute 'call' on every file on the path, until it returns 'false'.
			/// @return false if 'call' has returned false;
			bool for_each(std::function<bool (const char *filename)> call) {
				return for_each(c_str(),"*",false,call);
			}

			/// @brief Save file.
			static void save(const char *filename, const char *contents);

			/// @brief Save file to FD.
			static void save(int fd, const char *contents);

			/// @brief Replace file without backup
			static void replace(const char *filename, const char *contents);

			/// @brief Save file.
			inline void save(const char *contents) const {
				save(c_str(),contents);
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

				/// @brief Text length.
				size_t length;

				/// @brief Current position in the text.
				size_t	offset;

				/// @brief Value of the current row.
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
			Text(const std::string &filename) : Text(filename.c_str()) {
			}

			~Text();

			inline const char * c_str() const noexcept {
				return this->contents;
			}

			const Iterator begin() const noexcept;
			const Iterator end() const noexcept;

			Text & set(const char *contents);

			inline Text & set(const std::string &contents) {
				return set(contents.c_str());
			}

			static void for_each(const char *text, std::function<void (const std::string &line)> call);

			inline void for_each(std::function<void (const std::string &line)> call) const {
				for_each(contents,call);
			}

			inline void forEach(std::function<void (const std::string &line)> call) const {
				for_each(contents,call);
			}

			/// @brief Save file contents.
			void save() const;

			/// @brief Replace file contents without backup.
			/// @param filename The new filename.
			void replace(const char *filename);

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
			List(const char *path, const char *pattern, bool recursive=false);
			List(const char *path, bool recursive=false);

			List(const std::string &pattern, bool recursive=false) : List(pattern.c_str(),recursive) {
			}

			~List();

			/// @brief Navigate for all files until lambda returns 'false'.
			/// @return true if the lambda doesnt returns 'false' on any file.
			bool for_each(std::function<bool (const char *filename)> call);

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

		/// @brief Temporary file.
		class UDJAT_API Temporary {
		private:
			int fd = -1;

			/// @brief The reference filename.
			std::string filename;

#ifdef _WIN32
			Temporary();
			std::string tempname;
#endif // _WIN32

		public:
			/// @brief Create temporary file with same path of another one
			/// @param filename Filename to use as reference.
			Temporary(const char *filename);
			~Temporary();

			/// @brief Create an empty temporary file.
			static std::string create();

			/// @brief Create an empty temporary dir.
			static std::string mkdir();

#ifndef _WIN32

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

			/// @brief Write data to tempfile.
			/// @param contents Data to write.
			/// @param length Data length.
			Temporary & write(const void *contents, size_t length);

			inline Temporary & write(const std::string &str) {
				return write(str.c_str(),str.size());
			}

			inline Temporary & write(const char *str) {
				return write(str,strlen(str));
			}

			template<typename T>
			inline Temporary & write(const T &value) {
				return write((const void *) &value, sizeof(value));
			}

		};

	}

}

namespace std {

	template<typename T>
	inline Udjat::File::Temporary& operator<< (Udjat::File::Temporary &file, const T &value) {
			return file.write(value);
	}

}
