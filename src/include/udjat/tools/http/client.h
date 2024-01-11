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
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/url.h>
 #include <sstream>
 #include <memory>

 namespace Udjat {

	namespace HTTP {

		/// @brief Convenience 'get' method.
		/// @param url URL to get.
		/// @return String with response or exception.
		String get(const char *url);

		/// @brief Convenience 'save' method.
		/// @param url URL to get.
		/// @param filename File to save response.
		/// @return true if the file was updated.
		bool save(const char *url, const char *filename);

		/// @brief Simple HTTP client.
		class UDJAT_API Client {
		private:
			std::shared_ptr<Protocol::Worker> worker;

			/// @brief Request payload.
			std::ostringstream payload;

			/// @brief Default progress dialog.
			static const std::function<bool(double current, double total)> &default_progress;

		public:

			static void set(const std::function<bool(double current, double total)> progress);

			/// @brief Build client for URL.
			/// @param url The url.
			/// @param autoload If true try to load the required module.
			Client(const URL &url, bool autoload = false);

			Client(const XML::Node &node);

			Client(const char *url) : Client(URL(url)) {
			}

			Client(const std::string &url) : Client(URL(url)) {
			}

			inline Client & credentials(const char *user, const char *passwd) {
				worker->credentials(user,passwd);
				return *this;
			}

			template<typename T>
			inline Client & operator<<(T value) {
				payload << value;
				return *this;
			}

			inline std::string url() const {
				return worker->url();
			}

			inline int mimetype(const MimeType type) {
				return worker->mimetype(type);
			}

			/// @brief Setup cache headers from filename.
			/// @param filename The filename for cache information.
			void cache(const char *filename);

			/// @brief Get/Create request header.
			/// @param name Header name.
			/// @return The header.
			inline Protocol::Header & request(const char *name) {
				return worker->request(name);
			}

			/// @brief Get/Create response header.
			/// @param name Header name.
			/// @return The header.
			inline const Protocol::Header & response(const char *name) {
				return worker->response(name);
			}

			/// @brief Call URL, return response as string.
			String get(const std::function<bool(double current, double total)> &progress);

			/// @brief Call URL, return response as string.
			String get();

			/// @brief Call URL, return response as string.
			String post(const char *payload, const std::function<bool(double current, double total)> &progress);

			/// @brief Call URL, return response as string.
			String post(const char *payload);

			/// @brief Call URL, save response as filename.
			bool save(const char *filename, const std::function<bool(double current, double total)> &progress);

			/// @brief Call URL, save response as filename.
			/// @return true if the file was updated.
			bool save(const char *filename);

			/// @brief Save filename based on XML definitions.
			/// @param node XML node with URL & download settings.
			/// @return true if the file was updated.
			static bool save(const XML::Node &node, const char *filename, const std::function<bool(double current, double total)> &progress);

			/// @brief Save filename based on XML definitions.
			/// @param node XML node with URL & download settings.
			/// @return true if the file was updated.
			static bool save(const XML::Node &node, const char *filename);

			/// @brief Get URL, save response to cache file.
			/// @param progress The download progress notifier.
			/// @return The cached filename.
			std::string filename(const std::function<bool(double current, double total)> &progress);

			/// @brief Get URL, save response to cache file.
			/// @return The cached filename.
			std::string filename();

		};

	}

 }
