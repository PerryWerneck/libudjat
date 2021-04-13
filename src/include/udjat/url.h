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
 #include <memory>
 #include <string>
 #include <algorithm>

 namespace Udjat {

	class UDJAT_API URL {
	private:
		class Controller;

		class Protocol;
		frient class Protocol;

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
		static string unescape(const string &src);

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

			/// @brief Load URL.
			virtual void get(const URL &url, std::function<void(const char *block, size_t len)> reader);

			/// @brief Connect to URL, return socket.
			virtual int connect(const URL &url, time_t timeout = 0);

		};

		URL(const char *url);
		~URL();

	};


 }
