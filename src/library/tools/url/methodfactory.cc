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
 #include <udjat/defs.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/method.h>
 #include <stdexcept>
 #include <system_error>

 using namespace std;

 static const char * method_names[] = {
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"CONNECT",
	"OPTIONS",
	"TRACE",
	"PATCH",
 };

 namespace Udjat {

	HTTP::Method HTTP::MethodFactory(const char *name) {
		for(size_t ix = 0; ix < (sizeof(method_names)/sizeof(method_names[0])); ix++) {
			if(!strcasecmp(name,method_names[ix])) {
				return (Method) ix;
			}
		}
		throw system_error(EINVAL,system_category(),string{"The method '"} + name + "' is invalid");
	}

	HTTP::Method HTTP::MethodFactory(const XML::Node &node, const char *attrname, const char *def) {
		return MethodFactory(String{node,attrname,def}.c_str());
	}

	HTTP::Method HTTP::MethodFactory(const XML::Node &node, const char *def) {
		return MethodFactory(node,"http-method",def);
	}

	HTTP::Method HTTP::MethodFactory(const XML::Node &node) {
		return MethodFactory(node,"http-method","get");
	}

 }

 namespace std {

	const char * to_string(const Udjat::HTTP::Method method) {
		if((size_t) method > (sizeof(method_names)/sizeof(method_names[0]))) {
			throw system_error(EINVAL,system_category(),"Invalid method id");
		}
		return method_names[method];
	}

 }

