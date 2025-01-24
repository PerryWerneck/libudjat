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
 #include <udjat/tools/url.h>
 #include <udjat/tools/string.h>
 #include <uriparser/Uri.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	const String URL::scheme() const {

        UriUriA uri;
        const char * errorPos;
        if(uriParseSingleUriA(&uri, c_str(), &errorPos) != URI_SUCCESS) {
			throw std::invalid_argument("Invalid URL");
        }

		String result{uri.scheme.first, (size_t) (uri.scheme.afterLast - uri.scheme.first)};
		uriFreeUriMembersA(&uri);

		return result;
/*
		size_t pos = find("://");

		if(pos == string::npos) {
			return "";
		}

		return substr(0,pos);
*/
	}

	const URL::Handler & URL::handler() const {
		throw std::invalid_argument("Unsupported URL scheme");
	}


 }

