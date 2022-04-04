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
 #include <list>

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
		private:
			std::string field_name;

		public:
			Header(const char *n) : field_name(n) {
			}

			inline const char * name() const noexcept {
				return field_name.c_str();
			}

			inline const char * value() const noexcept {
				return this->c_str();
			}

			inline bool operator == (const char *name) const noexcept {
				return strcasecmp(name,this->field_name.c_str()) == 0;
			}

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
		private:
			struct Args{
				URL url;
				HTTP::Method method;

				Args(const URL &u, HTTP::Method m) : url(u), method(m) {
				}

			} args;

		protected:

			/// @brief Output data (To host)
			struct Out {
				std::string payload;	///< @brief Request payload.

				Out(const char *p) : payload(p) {
				}

			} out;

			/// @brief Input data (From host)
			struct In {
				TimeStamp modification;	///< @brief Last-modified time.
			} in;

		public:

			Worker(const char *url = "", const HTTP::Method method = HTTP::Get, const char *payload = "") : args(url, method), out(payload) {
			}

			Worker(const URL &url, const HTTP::Method method = HTTP::Get, const char *payload = "") : args(url, method), out(payload) {
			}

			/// @brief Set request credentials.
			virtual Worker & credentials(const char *user, const char *passwd);

			/// @brief Set request payload.
			inline Worker & payload(const char *payload) noexcept {
				out.payload = payload;
				return *this;
			}

			/// @brief Set request payload.
			inline Worker & payload(const std::string &payload) noexcept {
				out.payload = payload;
				return *this;
			}

			/// @brief Get request payload.
			inline const char * payload() const {
				return out.payload.c_str();
			}

			/// @brief Set request url.
			inline Worker & url(const char *url) noexcept {
				args.url = url;
				return *this;
			}

			/// @brief Set request url.
			inline Worker & url(const URL &url) noexcept {
				args.url = url;
				return *this;
			}

			inline const URL & url() const noexcept {
				return args.url;
			}

			inline Worker & method(const HTTP::Method method) noexcept {
				args.method = method;
				return *this;
			}

			inline HTTP::Method method() const noexcept {
				return args.method;
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
			/// @return true if the file was updated.
			virtual bool save(const char *filename, const std::function<bool(double current, double total)> &progress);

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

		inline const char * c_str() const noexcept {
			return name;
		}

		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(name,this->name) == 0;
		}

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



