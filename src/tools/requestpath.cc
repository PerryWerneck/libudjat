/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Brief description of this source.
  */

 #include <config.h>
 #include <udjat/tools/abstract/request-path.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	RequestPath::RequestPath(const XML::Node &node)
		:	object_name{Quark{node,"name",nullptr}.c_str()},
			object_path{Quark{node,"path",object_name}.c_str()},
			method{HTTP::MethodFactory(node,"action","get")},
			expires((time_t) TimeStamp{node,"expires",(time_t) 60}) {

		if(object_path[0] == '/') {
			throw system_error(EINVAL,system_category(),"API path cant start with '/'");
		}

	}

	bool RequestPath::head(const Request &, Abstract::Response &response) const {

		// TODO: Check for 'not modified' response.

		if(expires) {
			response.expires(time(0)+expires);
		}

		return false;
	}

	bool RequestPath::operator==(const char *request_path) const noexcept {

		if(*request_path == '/') {
			request_path++;
		}

		size_t szPath = strlen(object_path);

		debug("path='",object_path,"' request='",request_path,"' szPath=",szPath);

		return szPath <= strlen(request_path) && (request_path[szPath] == '/' || request_path[szPath] == 0) && strncasecmp(request_path,object_path,szPath) == 0;

	}

	bool RequestPath::operator==(const Request &request) const noexcept {

		debug("------------------------------------------------------");

		debug(name()," method is",((request == method) ? "equal" : "not equal"));

		return request == method && *this == request.path();
	}

 }
