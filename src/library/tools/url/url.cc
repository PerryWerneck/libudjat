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
 #include <udjat/tools/logger.h>
 #include <uriparser/Uri.h>
 #include <stdexcept>
 #include <string>
 #include <list>

 using namespace std;

 namespace Udjat {



	const String URL::servicename() const {
		
		UriUriA uri;
		const char * errorPos;
		if(uriParseSingleUriA(&uri, c_str(), &errorPos) != URI_SUCCESS) {
			throw std::invalid_argument("Invalid URL");
		}

		String result{uri.portText.first, (size_t) (uri.portText.afterLast - uri.portText.first)};
		uriFreeUriMembersA(&uri);

		if(result.empty()) {
			return scheme();
		}

		return result;
	}

	const String URL::hostname() const {
		
		UriUriA uri;
		const char * errorPos;
		if(uriParseSingleUriA(&uri, c_str(), &errorPos) != URI_SUCCESS) {
			throw std::invalid_argument("Invalid URL");
		}

		String result{uri.hostText.first, (size_t) (uri.hostText.afterLast - uri.hostText.first)};
		uriFreeUriMembersA(&uri);

		return result;
	}

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

	URL & URL::operator += (const char *path) {

		UriUriA uri;
		UriParserStateA state;
		state.uri = &uri;

		if (uriParseUriA(&state, c_str()) != URI_SUCCESS) {
			uriFreeUriMembersA(&uri);
			throw std::invalid_argument("Invalid URL");
		}

		std::string newUri;

		// Scheme
		if (uri.scheme.first) {
			newUri.append(uri.scheme.first, uri.scheme.afterLast - uri.scheme.first);
			newUri.append("://");
		}

		// Hostname
		if (uri.hostText.first) {
			newUri.append(uri.hostText.first, uri.hostText.afterLast - uri.hostText.first);
		}

		if (uri.portText.first) {
			newUri.append(":");
			newUri.append(uri.portText.first, uri.portText.afterLast - uri.portText.first);
		}

		// Path
		std::list<string> segments;
		if (uri.pathHead) {
			for(UriPathSegmentA *pathSegment = uri.pathHead; pathSegment; pathSegment = pathSegment->next) {
				segments.push_back(string{pathSegment->text.first, pathSegment->text.afterLast - pathSegment->text.first});
			}
		}

		const char *ptr = path;
		while(*ptr == '.') {
			if(!strncmp(ptr,"./",2)) {
				// Just extract.
				ptr += 2;
			} else if(!strncmp(ptr,"../",3)) {
				// Remove last segment.
				if(!segments.empty()) {
					segments.pop_back();
				}
				ptr += 3;
			} else {
				break;
			} 
		}

		// Concatenate path.
		for(const string &segment : segments) {
			newUri.append("/");
			newUri.append(segment);
		}
		
		if(*ptr == '/') {
			ptr++;
		}
		
		if(*ptr) {
			newUri.append("/");
			newUri.append(ptr);
		}

		uriFreeUriMembersA(&uri);

		*this = newUri;

		return *this;
	}

	const URL::Handler & URL::handler() const {
		throw std::invalid_argument("Unsupported URL scheme");
	}


 }

