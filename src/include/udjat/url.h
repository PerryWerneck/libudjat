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

 /**
  * @brief Declares URL object.
  *
  * References:
  *
  * <https://www.algosome.com/articles/anatomy-of-website-url.html>
  *
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/request.h>
 #include <udjat/tools/quark.h>
 #include <memory>
 #include <string>
 #include <algorithm>

 namespace Udjat {

	class UDJAT_API URL {
	public:

		/// HTTP Request methods.
		/// <https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods>
		class UDJAT_API Method {
		public:
			enum Value : uint8_t {
				Get,		///< @brief Requests a representation of the specified resource.
				Head,		///< @brief Asks for a response identical to that of a GET request, but without the response body.
				Post,		///< @brief Submit an entity to the specified resource, often causing a change in state or side effects on the server.
				Put,		///< @brief Replaces all current representations of the target resource with the request payload.
				Delete,		///< @brief Deletes the specified resource.
				Connect,	///< @brief Establishes a tunnel to the server identified by the target resource.
				Options,	///< @brief Describe the communication options for the target resource.
				Trace,		///< @brief Performs a message loop-back test along the path to the target resource.
				Patch,		///< @brief Apply partial modifications to a resource.
			} value;

			constexpr Method() : value(Get) {
			}

			constexpr Method(Value method) : value(method) {
			}

			Method(const char *name);

			Method & operator = (const char *name);

			operator Value() const noexcept {
				return value;
			}

			const char *c_str() const noexcept;

			inline operator const char *() const noexcept {
				return c_str();
			}

			explicit operator bool() = delete;

			constexpr bool operator==(Method a) const {
				return value == a.value;
			}

			constexpr bool operator!=(Method a) const {
				return value != a.value;
			}
		};

		/// @brief URL module worker.
		class UDJAT_API Protocol {
		private:

			/// @brief The protocol name.
			Quark name;

		protected:

			/// @brief The default port name.
			const char *portname;

			/// @brief Module information.
			const ModuleInfo *info;

		public:
			Protocol(const Quark &protocol, const char *portname = "");
			virtual ~Protocol();

			inline const char * c_str() const {
				return name.c_str();
			}

			inline const ModuleInfo * getModuleInfo() const noexcept {
				return this->info;
			}

			/// @brief Get default port name;
			inline const char * getDefaultPortName() const noexcept {
				return portname;
			}

			/// @brief Connect to URL.
			/// @return Socket connected to host.
			virtual int connect(const URL &url, time_t timeout);

			/// @brief Call protocol method.
			/// @param url The URL to call.
			/// @param method Required method.
			/// @param Mimetype
			/// @param payload URL payload.
			/// @return String with the host response.
			virtual std::string call(const URL &url, const Method method, const char *mimetype, const char *payload = nullptr);

			/// @brief Call protocol method.
			/// @param url The URL to call.
			/// @param method Required method.
			/// @param payload URL payload.
			/// @return Host response.
			Response call(const URL &url, const Method method, const Request &payload);

		};


	private:
		class Controller;

		friend class Protocol;

		/// @brief The URL protocol.
		std::shared_ptr<Protocol> protocol;

		/// @brief The URL Domain name.
		std::string domain;

		/// @brief The URL port.
		std::string port;

		/// @brief The URL filename.
		std::string filename;

	public:

		/// @brief Unescape URL
		static std::string unescape(const char *src);

		/// @brief Insert protocol back-end.
		static void insert(std::shared_ptr<Protocol> p);

		/// @brief get list of installed protocols.
		static void getInfo(Response &response);

		URL();
		URL(const char *url);
		~URL();

		/// @brief Assign value to URL
		URL & assign(const char *url);

		/// @brief Get URL as string
		std::string to_string() const;

		/// @brief Connect to host.
		/// @return Socket connected to host.
		int connect(time_t timeout = 0) const;

		/// @brief Get
		std::string get(const char *mimetype = "");

		/// @brief Post
		std::string post(const char *payload, const char *mimetype = "");

	};


 }

 /*
 namespace std {

	inline string to_string(const Udjat::URL &url) {
		return url.get();
	}

 }
 */
