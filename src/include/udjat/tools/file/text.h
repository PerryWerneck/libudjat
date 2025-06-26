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
#include <udjat/tools/file/path.h>

namespace Udjat {

	namespace File {

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

			/// @brief Expand ${} macros.
			/// @param expander value expander method.
			/// @param dynamic if true expands the dynamic values like ${timestamp(format)}.
			/// @param cleanup if true remove the non existent values from string.
			Text & expand(const std::function<bool(const char *key, std::string &str)> &expander, bool dynamic = false, bool cleanup = false);

#if __cplusplus >= 201703L
			/// @brief Expand using customized marker.
			/// @param expander value expander method.
			/// @param marker The marker.
			/// @param dynamic if true expands the dynamic values like ${timestamp(format)}.
			/// @param cleanup if true remove the non existent values from string.
			Text & expand(char marker, const std::function<bool(const char *key, std::string &str)> &expander, bool dynamic = false, bool cleanup = false);

			Text & expand(char marker, bool dynamic = true, bool cleanup = true);

			inline Text & expand(bool dynamic = true, bool cleanup = true) {
				return expand('$',dynamic,cleanup);
			}
#endif // __cplusplus >= 201703L
			
			/// @brief Save file contents.
			void save() const;

			/// @brief Save file contents with a new name.
			void save(const char *name);

			/// @brief Replace file contents without backup.
			/// @param filename The new filename.
			void replace(const char *filename);

		};

	}

}
