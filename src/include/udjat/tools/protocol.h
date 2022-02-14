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
 #include <udjat/request.h>

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

		/// @brief Call protocol method.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		static std::string call(const char *url, const HTTP::Method method, const char *payload = "");

		/// @brief Call protocol method.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		virtual std::string call(const URL &url, const HTTP::Method method, const char *payload = "") const;

		/// @brief Call protocol method.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		std::string call(const URL &url, const char *method, const char *payload = "") const;

		/// @brief Download/update file.
		/// @param url the file URL.
		/// @param filename The fullpath for the file.
		/// @param progress The progress callback.
		/// @return true if the file was updated.
		virtual bool get(const URL &url, const char *filename, const std::function<bool(double current, double total)> &progress) const;


	};

 }



