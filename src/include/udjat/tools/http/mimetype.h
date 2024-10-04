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
 #include <ostream>
 #include <cstdint>

 namespace Udjat {

	enum MimeType : uint8_t {
		custom,					///> @brief Custom
		json,                   ///> @brief application/json; charset=utf-8
		csv,                    ///> @brief text/csv; charset=utf-8
		tsv,                    ///> @brief text/tab-separated-values; charset=utf-8
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
		gz,						///> @brief application/gzip
		rpm,					///> @brief application/x-rpm
		odt,					///> @brief application/vnd.oasis.opendocument.text
		ods,					///> @brief application/vnd.oasis.opendocument.spreadsheet
		sh,						///> @brief application/x-sh
		pdf,					///> @brief application/pdf

		// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
		rtf,					///> @brief application/rtf
		xhtml,					///> @brief application/xhtml+xml
		zip,					///> @brief application/zip
		cacert,					///> @brief application/x-x509-ca-cert

		form_urlencoded,		///> @brief application/x-www-form-urlencoded

	};

	/// @brief Create mimetype from string.
	/// @param str Mime type string.
	/// @param log_def enable log message when using default value.
	UDJAT_API MimeType MimeTypeFactory(const char *str, bool log_def = true) noexcept;

	/// @brief Create mimetype from string with fallback.
	/// @param str Mime type string.
	/// @param def Default value if not found.
	UDJAT_API MimeType MimeTypeFactory(const char *str, const MimeType def) noexcept;

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::MimeType type, bool suffix = false);

	inline ostream& operator<< (ostream& os, Udjat::MimeType type) {
			return os << std::to_string(type);
	}

 }
