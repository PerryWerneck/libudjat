/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
#include <string>
#include <vector>
#include <functional>

namespace Udjat {

	namespace System {

		namespace Config {

			/// @brief SysConfig file parser.
			class UDJAT_API File {
			public:

				/// @brief SysConfig Value
				class UDJAT_API Value {
				public:

					/// @brief The input type.
					enum Type {
						Boolean,
						Integer,
						String
					} type = String;

					/// @brief The default value.
					std::string def = "";

					/// @brief The value name.
					std::string name = "";

					/// @brief The value.
					std::string value = "";

					void clear();
					void setType(const char *name);

					/// @brief Convenience method for agent 'get()' method.
					Udjat::Value & get(Udjat::Value &value) const;

					operator bool() const noexcept {
						return !value.empty();
					}

					operator const char *() const {
						return value.c_str();
					}

				};

			private:

				/// @brief The file description.
				std::string description;

				/// @brief The file path
				std::string path;

				/// @brief The key/value delimiter
				char separator;

				/// @brief File contents.
				std::vector<Value> values;

			public:

				File();
				File(const char *filename, const char *separator = "\0");
				File & set(const char *contents);

				Value find(const char *key) const noexcept;

				Value operator[](const char *key) const noexcept {
					return find(key);
				}

				/// @brief Get the object name.
				/// @return String with the filename without path or extension.
				std::string name() const;

				const char * getPath() const noexcept {
					return path.c_str();
				}

				const char * getDescription() const noexcept {
					return description.c_str();
				}

				void forEach(std::function<void(const Value &value)> callback) const;

				inline std::vector<Value>::const_iterator begin() const {
					return values.begin();
				}

				inline std::vector<Value>::const_iterator end() const {
					return values.end();
				}

			};

		}

	}

}