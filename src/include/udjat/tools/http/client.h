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
 #include <udjat/tools/url.h>
 #include <udjat/tools/protocol.h>

 namespace Udjat {

	namespace HTTP {

		/// @brief Simple HTTP client.
		class UDJAT_API Client {
		private:
			std::shared_ptr<Protocol::Worker> worker;

			/// @brief Request payload.
			std::ostringstream payload;

		public:
			Client(const char *url);
			Client(const std::string &url);

			template<typename T>
			inline Client & operator<<(T value) {
				payload << value;
				return *this;
			}

			/// @brief Get Header.
			/// @param name Header name.
			/// @return Header info.
			inline Protocol::Header & header(const char *name) {
				return worker->header(name);
			}

			/// @brief Get header.
			/// @param key The header name.
			/// @return The header.
			inline Protocol::Header & operator[](const char *name) {
				return worker->header(name);
			}

			/// @brief Call URL, return response as string.
			inline String get(const std::function<bool(double current, double total)> &progress) {
				return worker->get(progress);
			}

			/// @brief Call URL, save response as filename.
			inline bool save(const char *filename, const std::function<bool(double current, double total)> &progress) {
				return worker->save(filename,progress);
			}

			/// @brief Call URL, return response as string.
			inline String get() {
				return worker->get();
			}

			/// @brief Call URL, save response as filename.
			/// @return true if the file was updated.
			inline bool save(const char *filename) {
				return worker->save(filename);
			}

		};

	}

 }
