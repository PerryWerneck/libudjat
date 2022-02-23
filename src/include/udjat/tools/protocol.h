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
 #include <udjat/tools/string.h>
 #include <udjat/request.h>
 #include <udjat/tools/timestamp.h>

 namespace Udjat {

	/// @brief Network protocol module.
	class UDJAT_API Protocol {
	private:
		class Controller;
		friend class Controller;

		/// @brief The protocol name.
		const char * name;

		/// @brief Module information.
		const ModuleInfo &module;

	public:

		/// @brief Request header.
		class UDJAT_API Header : public std::string {
		public:
			virtual Header & assign(const TimeStamp &value);

			Header & assign(const char *value);
			Header & assign(const std::string &value);

			template <typename T>
			Header assign(const T value) {
				return assign(std::to_string(value));
			}

			Header & operator = (const TimeStamp &value) {
				return assign(value);
			}

			template <typename T>
			Header & operator = (const T value) {
				return assign(std::to_string(value));
			}

		};

		/// @brief Protocol Worker.
		class UDJAT_API Worker {
		protected:
			struct {
				URL url;
				HTTP::Method method;
				std::string payload;
			} args;

		public:
			Worker(const char *url = "", const HTTP::Method method = HTTP::Get, const char *payload = "");
			virtual ~Worker();

			inline void payload(const char *payload) noexcept {
				args.payload = payload;
			}

			inline void url(const char *url) noexcept {
				args.url.assign(url);
			}

			inline void url(const URL &url) noexcept {
				args.url = url;
			}

			inline void method(const HTTP::Method method) noexcept {
				args.method = method;
			}

			/// @brief Get Header.
			/// @param name Header name.
			/// @return Header info.
			virtual Header & header(const char *name);

			/// @brief Get header.
			/// @param key The header name.
			/// @return The header.
			inline Header & operator[](const char *name) {
				return header(name);
			}

			/// @brief Set request header.
			/// @param name Header name.
			/// @param value Header value;
			inline void header(const char *name, const char *value) {
				header(name).assign(value);
			}

			/// @brief Set request header.
			/// @param name Header name.
			/// @param value Header value;
			template <typename T>
			inline void header(const char *name, const T value) {
				header(name).assign(value);
			}

			/// @brief Call URL, return response as string.
			virtual String get(const std::function<bool(double current, double total)> &progress) = 0;

			/// @brief Call URL, save response as filename.
			virtual bool save(const char *filename, const std::function<bool(double current, double total)> &progress) = 0;

			/// @brief Call URL, return response as string.
			String get();

			/// @brief Call URL, save response as filename.
			/// @return true if the file was updated.
			bool save(const char *filename);

		};

		virtual std::shared_ptr<Worker> WorkerFactory() const;

		Protocol(const Protocol &) = delete;
		Protocol(const Protocol *) = delete;

		Protocol(const char *name, const ModuleInfo &module);
		virtual ~Protocol();

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;

		static const Protocol * find(const URL &url);
		static const Protocol * find(const char *name);

		static void getInfo(Udjat::Response &response) noexcept;

		/// @brief Find protocol and call.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		static String call(const char *url, const HTTP::Method method = HTTP::Get, const char *payload = "");

		/// @brief Call protocol method.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		virtual String call(const URL &url, const HTTP::Method method, const char *payload = "") const;

		/// @brief Call protocol method.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		String call(const URL &url, const char *method, const char *payload = "") const;

		/// @brief Download/update file.
		/// @param url the file URL.
		/// @param filename The fullpath for the file.
		/// @param progress The progress callback.
		/// @return true if the file was updated.
		virtual bool get(const URL &url, const char *filename, const std::function<bool(double current, double total)> &progress) const;


	};

 }



