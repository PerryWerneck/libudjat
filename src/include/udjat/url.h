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

		/// @brief URL module worker.
		class UDJAT_API Protocol {
		private:

			/// @brief The protocol name.
			Quark name;

		protected:

			/// @brief The default port name.
			const char *portname;

			Protocol(const Quark &protocol);

		public:
			virtual ~Protocol();

			/// @brief Get default port name;
			inline const char * getDefaultPortName() const noexcept {
				return portname;
			}

			/// @brief Get URL
			virtual void get(const URL &url, time_t timeout, std::function<void(const char *block, size_t len)> reader);

			/// @brief Get URL
			virtual void get(const URL &url, time_t timeout, Response &response);

			/// @brief Connect to URL.
			/// @return Socket connected to host.
			virtual int connect(const URL &url, time_t timeout);

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

		URL();
		URL(const char *url);
		~URL();

		/// @brief Assign value to URL
		URL & assign(const char *url);

		/// @brief get string from URL.
		/// @return String with URL response.
		std::string get(time_t timeout = 0) const;

		/// @brief get URL.
		/// @param response Objecto for URL response.
		void get(Response &response, time_t timeout = 0) const;

		/// @brief Connect to host.
		/// @return Socket connected to host.
		int connect(time_t timeout = 0) const;

	};


 }

 /*
 namespace std {

	inline string to_string(const Udjat::URL &url) {
		return url.get();
	}

 }
 */
