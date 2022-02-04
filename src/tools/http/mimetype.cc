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

 #include <udjat/defs.h>
 #include <udjat/tools/http/mimetype.h>
 #include <cstring>
 #include <iostream>

 using namespace std;

 static const struct {
	const char *ext;
	const char *str;
 } types[] = {

 	// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
	{ "dat",	"application/octet-stream" },

	{ "json",	"application/json; charset=utf-8" },
	{ "csv",	"text/csv; charset=utf-8" },
	{ "tsv",	"text/csv; charset=utf-8" },
	{ "txt",	"text/plain; charset=utf-8" },
	{ "xml",	"text/xml; charset=utf-8" },
	{ "html",	"text/html; charset=utf-8" },


	{ "css",	"text/css; charset=utf-8" },
	{ "js",		"application/javascript" },
	{ "svg",	"image/svg+xml" },
	{ "gif",	"image/gif" },
	{ "jpg",	"image/jpeg" },
	{ "png",	"image/png" },
	{ "pem",	"application/x-pem-file" },
	{ "ico",	"image/x-icon" },

	{ "yaml",	"text/yaml" }, // https://stackoverflow.com/a/332159/2356331


 };

 const char * std::to_string(const Udjat::MimeType type, bool suffix) {

	size_t ix = (size_t) type;

	if(ix > (sizeof(types)/sizeof(types[0])))
		ix = 0;

	return (suffix ? types[ix].ext : types[ix].str);
 }

 Udjat::MimeType Udjat::MimeTypeFactory(const char *str) noexcept {

	for(size_t ix = 0; ix < (sizeof(types)/sizeof(types[0])); ix++) {
		if(!strcasecmp(str,types[ix].str)) {
			return (MimeType) ix;
		}
 	}

	for(size_t ix = 0; ix < (sizeof(types)/sizeof(types[0])); ix++) {
		if(!strcasecmp(str,types[ix].ext)) {
			return (MimeType) ix;
		}
 	}

 	clog << "http\tUnknown mimetype '" << str << "' assuming '" << types[MimeType::custom].str << "'" << endl;

 	return MimeType::custom;
 }
