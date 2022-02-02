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
 #include <string>

 namespace Udjat {

	enum MimeType : uint8_t {
		custom,					///> @brief Custom
		json,                   ///> @brief application/json; charset=utf-8
		csv,                    ///> @brief text/csv; charset=utf-8
		tsv,                    ///> @brief text/csv; charset=utf-8
		text,                   ///> @brief text/plain; charset=utf-8
		xml,                    ///> @brief text/xml; charset=utf-8
		html,                   ///> @brief text/html; charset=utf-8

		css,                    ///> @brief text/css; charset=utf-8
		javascript,             ///> @brief application/javascript

		svg,                    ///> @brief image/svg+xml
		gif,                    ///> @brief image/gif
		jpg,                    ///> @brief image/jpeg
		png,                    ///> @brief image/png
		pem,                    ///> @brief application/x-pem-file
		icon,                   ///> @brief image/x-icon
		yaml,					///> @brief text/yaml

	};

	UDJAT_API MimeType MimeTypeFactory(const char *str) noexcept;

 }

 namespace std {

	const char * to_string(const Udjat::MimeType type, bool suffix = false);

	inline ostream& operator<< (ostream& os, Udjat::MimeType type) {
			return os << std::to_string(type);
	}

 }
