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
 #include <udjat/alert.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/abstract/object.h>
 
 namespace Udjat {

	/// @brief Alert updating a file when activated.
	class UDJAT_API URLAlert : public Alert {
	protected:

		const char *url = "";
		HTTP::Method action = HTTP::Get;

		struct Payload {
			const char *tmpl;		///< @brief Template to payload.
			String value;			///< @brief Current payload.

			Payload(const char *t = "") : tmpl{t} {
			}

		} payload;

		void reset(time_t next = 0) noexcept override;

		int emit() override;

	public:

		URLAlert(const XML::Node &node);

		URLAlert(const char *name, const char *u, const HTTP::Method a = HTTP::Get, const char *p = "") 
			: Alert{name}, url{u}, action{a}, payload{p} {
		}

		virtual ~URLAlert();

		bool activate() noexcept override;
		bool activate(const Udjat::Abstract::Object &object) noexcept override;

	};
 
 }
