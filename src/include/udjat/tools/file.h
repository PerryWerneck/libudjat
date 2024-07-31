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
#include <udjat/tools/file/handler.h>
#include <udjat/tools/file/temporary.h>
#include <list>
#include <algorithm>
#include <mutex>
#include <string>
#include <cstring>
#include <cstdint>
#include <sys/stat.h>
#include <udjat/tools/string.h>

namespace Udjat {

	namespace File {

		using Stat = struct stat;

		class Agent;
		class Controller;

		/// @brief Copy file
		UDJAT_API void copy(int from, const char *to);

		inline void save(int from, const char *to) {
			copy(from,to);
		}

		/// @brief Copy file with custom writer.
		UDJAT_API void copy(const char *from, const std::function<void(unsigned long long offset, unsigned long long total, const void *buf, size_t length)> &writer);

		/// @brief Copy file
		UDJAT_API void copy(const char *from, const char *to, bool replace = true);

		UDJAT_API void copy(const char *from, const char *to, const std::function<bool(double current, double total)> &progress, bool replace = true);

		/// @brief Set file modification time.
		/// @return 0 if ok, non zero if not (sets errno).
		UDJAT_API int mtime(const char *filename, time_t time);

#ifdef _WIN32

		UDJAT_API int move(const char *from, const char *to, bool replace = false);

#else
		/// @brief Move file, create 'bak'
		/// @param from The origin file name.
		/// @param to The target file name.
		/// @param no_backup if false '.bak' file will be created with old contents.
		UDJAT_API int move(const char *from, const char *to, bool no_backup = false);

		/// @brief Move file, create 'bak'
		/// @param The handle of an open origin file.
		/// @param to The target file name.
		/// @param no_backup if false '.bak' file will be created with old contents.
		UDJAT_API int move(int fd, const char *to, bool no_backup = false);

#endif // _WIN32


		/// @brief Save to temporary file.
		/// @param contents String with file contents.
		/// @return Temporary file name.
		UDJAT_API std::string save(const char *contents);

		/// @brief File Path.
		class UDJAT_API Path : public std::string {
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

			Path(const std::string &v) : Path(v.c_str()) {
			}

			static void expand(std::string &str);

			inline void expand() {
				expand(*this);
			}

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
		/*
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
		*/

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

	}

}
