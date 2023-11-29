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

 #include <config.h>
 #include <private/request.h>
 #include <cstring>
 #include <cstdarg>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/configuration.h>

 namespace Udjat {

	/*
	static const std::string sanitize(const char *ptr) {

		if(ptr) {
			const char *mark = strstr(ptr,"://");
			if(mark) {
				ptr = mark + 3;
			}

			while(*ptr && *ptr == '/') {
				ptr++;
			}
		}

		const char *args = strchr(ptr,'?');
		if(args) {
			return String{ptr,args-ptr}.strip();
		}

		return String{ptr}.strip();
	}
	*/

	Request::Request(const HTTP::Method method) : request{method} {
	}

	Request::Request(const char *method) : Request{HTTP::MethodFactory(method)} {
	}

	String Request::getProperty(const char *, const char *def) const {
		return def;
	}

	/// @brief Get request property by index.
	/// @param index The property index
	/// @param def The default value.
	String Request::getProperty(size_t, const char *def) const {
		return def;
	}

	String Request::pop() {

		if(!(request.popptr && *request.popptr)) {
			return "";
		}

		const char *next = strchr(request.popptr,'/');
		if(next) {
			String rc{request.popptr,next-request.popptr};
			request.popptr = next+1;
			return rc;
		}

		string rc{request.popptr};
		request.popptr = "";
		return rc;

	}

	Request & Request::pop(std::string &value) {

		value = pop();
		return *this;

	}

	Request & Request::pop(int &value) {
		value = stoi(pop());
		return *this;
	}

	Request & Request::pop(unsigned int &value) {
		value = (unsigned int) stoi(pop());
		return *this;
	}

	const char * Request::c_str() const noexcept {
		Logger::String{"Processing request with no path"}.warning("request");
		return "";
	}

	Request & Request::rewind(bool required_versioned_path) {

		static const struct {
			const char *value;
			MimeType mimetype;
		} mimetypes[] {
			{ "json/",	MimeType::json	},
			{ "html/",	MimeType::html	},
			{ "xml/",	MimeType::xml	},
			{ "yaml/",	MimeType::yaml	},
			{ "csv/",	MimeType::csv	},
			{ "sh/",	MimeType::sh	},
		};

		request.popptr = c_str();

		{
			const char *mark = strstr(request.popptr,"://");
			if(mark) {
				request.popptr = mark + 3;
			}
		}

		while(*request.popptr && *request.popptr == '/') {
			request.popptr++;
		}

		if(!strncasecmp(request.popptr,"api/",4)) {

			// It's an standard API method.
			request.popptr += 4;

			while(*request.popptr && *request.popptr != '/') {
				if(isdigit(*request.popptr)) {
					apiver *= 10;
					apiver += (*request.popptr - '0');
				}
				request.popptr++;
			}

			// Do we have mimetype on URL?
			for(const auto &entry : mimetypes) {
				size_t szValue = strlen(entry.value);
				if(*request.popptr == '/' && !strncasecmp(request.popptr+1,entry.value,szValue)) {
					request.popptr += szValue;
					this->mimetype = entry.mimetype;
					debug("Mimetype set to '",this->mimetype,"' from URL");
					break;
				}
			}

			debug("API Version set to '",apiver,"'");

		} else if(required_versioned_path) {

			throw HTTP::Exception(400,"Request path should be /api/[VERSION]/[REQUEST]");

		} else {

			// Unversioned path

			apiver = 0;

			for(const auto &entry : mimetypes) {
				size_t szValue = strlen(entry.value);
				if(!strncasecmp(request.popptr,entry.value,szValue)) {
					request.popptr += szValue;
					this->mimetype = entry.mimetype;
					debug("Mimetype set to '",this->mimetype,"' from legacy path");
					break;
				}
			}

		}

		// Start on first argument.
		while(*request.popptr && *request.popptr == '/') {
			request.popptr++;
		}

		Logger::String{"Effective request path is '",request.popptr,"'"}.trace("request");

		return *this;
	}

 }

