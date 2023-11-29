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
		rewind();
	}

	Request::Request(const char *method) : Request{HTTP::MethodFactory(method)} {
	}

	String Request::getProperty(const char *name, const char *def) const {
		return def;
	}

	/// @brief Get request property by index.
	/// @param index The property index
	/// @param def The default value.
	String Request::getProperty(size_t index, const char *def) const {
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

	Request & Request::rewind() {

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

			for(const auto &entry : mimetypes) {
				size_t szValue = strlen(entry.value);
				if(!strncasecmp(request.popptr,entry.value,szValue)) {
					request.popptr += szValue;
					this->mimetype = entry.mimetype;
					break;
				}
			}

			debug("API Version set to '",apiver,"'");
		}

		// Start on first argument.
		while(*request.popptr && *request.popptr == '/') {
			request.popptr++;
		}

		Logger::String{"Effective request path is '",request.popptr,"'"}.trace("request");

		return *this;
	}

 }

