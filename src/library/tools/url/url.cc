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
 #include <memory>

 using namespace std;

 namespace Udjat {

	struct ParsedUri : UriUriA {
		ParsedUri(const std::string &str) {
			const char * errorPos;
			if(uriParseSingleUriA(this, str.c_str(), &errorPos) != URI_SUCCESS) {
				throw std::invalid_argument("Invalid URL");
			}
		}

		~ParsedUri() {
			uriFreeUriMembersA(this);
		}
	};

	const String URL::servicename() const {
		
		ParsedUri uri{*this};

		String result{uri.portText.first, (size_t) (uri.portText.afterLast - uri.portText.first)};

		if(result.empty()) {
			return scheme();
		}

		return result;
	}

	const String URL::hostname() const {
		
		ParsedUri uri{*this};
		return String{uri.hostText.first, (size_t) (uri.hostText.afterLast - uri.hostText.first)};

	}

	const String URL::scheme() const {

		ParsedUri uri{*this};
		return String{uri.scheme.first, (size_t) (uri.scheme.afterLast - uri.scheme.first)};

/*
		size_t pos = find("://");

		if(pos == string::npos) {
			return "";
		}

		return substr(0,pos);
*/
	}

	URL & URL::operator += (const char *path) {

		ParsedUri uri{*this};

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
				size_t len =  (size_t) (pathSegment->text.afterLast - pathSegment->text.first);
				segments.emplace_back(pathSegment->text.first,len);
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
				throw invalid_argument("Bad format");
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

		if(uri.query.first) {
			newUri.append("?");
			newUri.append(uri.query.first, uri.query.afterLast - uri.query.first);
		}
	
		*this = newUri;

		return *this;
	}

	bool URL::for_each(const std::function<bool(const char *key, const char *value)> &func) const {
		ParsedUri uri{*this};
		UriQueryListA *queryList = nullptr;
		int items = 0; 
		bool rc = false;

		if(!uri.query.first) {
			return false;
		}

		if(uriDissectQueryMallocA(&queryList, &items, uri.query.first, uri.query.afterLast) != URI_SUCCESS) {
			throw runtime_error("Unexpected error on uriDissectQueryMallocA");
		}

		for (UriQueryListA *node = queryList; node && !rc; node = node->next) {
			rc = func(node->key, node->value);
		}

		uriFreeQueryListA(queryList);

		return rc;
	}

	String URL::argument(const char *name) const {

		String result;

		for_each([&result,name](const char *key, const char *value) -> bool {
			if(!strcasecmp(key,name)) {
				result = value;
				return true;
			}
			return false;
		});

    	return result;

	}


 }

