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
 
 namespace Udjat {

	/// @brief Alert updating a file when activated.
	class UDJAT_API FileAlert : public Alert {
	protected:

		const char *filename = "";	///< @brief File to update.
		time_t maxage = 86400;		///< @brief Maximum age for the file.

		struct Payload {
			const char *tmpl;		///< @brief Template to payload.
			String value;			///< @brief Current payload.

			Payload(const char *t = "") : tmpl{t} {
			}

		} payload;

		void reset(bool active) noexcept override;

		int emit() override;

	public:

		FileAlert(const char *name, const char *f, const char *p = "") : Alert{name}, filename{f}, payload{p} {
		}

		FileAlert(const XML::Node &node);

		virtual ~FileAlert();

		bool activate() noexcept override;
		bool activate(const Udjat::Abstract::Object &object) noexcept override;

	};

 }
 