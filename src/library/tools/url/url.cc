/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <sstream>
 #include <netdb.h>
 #include <private/urlparser.h>
 #include <libgen.h>

 using namespace std;

 namespace Udjat {

	String URL::servicename() const {
		
		ParsedUri uri{*this};

		String result{uri.portText.first, (size_t) (uri.portText.afterLast - uri.portText.first)};

		if(result.empty()) {
			return scheme();
		}

		return result;
	}

	int URL::port(const char *proto) const {

		#include <udjat/tools/string.h>

		ParsedUri uri{*this};
		String result{uri.portText.first, (size_t) (uri.portText.afterLast - uri.portText.first)};

		if(result.empty()) {
			result = scheme();
		} else if(result.isnumber()) {
			return atoi(result.c_str());
		}

		struct servent *service = getservbyname(result.c_str(),proto);
		if (service) {
			return ntohs(service->s_port);
		}

		throw system_error(ENXIO, system_category(), String{"Service '",result.c_str(),"' not found"});

	}

	String URL::hostname() const {
		
		ParsedUri uri{*this};
		return String{uri.hostText.first, (size_t) (uri.hostText.afterLast - uri.hostText.first)};

	}

	String URL::scheme() const {

		ParsedUri uri{*this};
		return String{uri.scheme.first, (size_t) (uri.scheme.afterLast - uri.scheme.first)};
	}

	String URL::path() const {

		ParsedUri uri{*this};

		String result;

		if (uri.pathHead) {
			for(UriPathSegmentA *pathSegment = uri.pathHead; pathSegment; pathSegment = pathSegment->next) {
				result += '/';
				size_t len =  (size_t) (pathSegment->text.afterLast - pathSegment->text.first);
				result += string{pathSegment->text.first,len};
			}
		}

		return result;
	}

	String URL::name() const {

		String path{this->path()};

		char buffer[path.size()+1];
		strncpy(buffer,path.c_str(),path.size());
		buffer[path.size()] = 0;

		return basename(buffer);

	}

	String URL::dirname() const {
		
		String path{this->path()};

		char buffer[path.size()+1];
		strncpy(buffer,path.c_str(),path.size());
		buffer[path.size()] = 0;

		return ::dirname(buffer);
	}

	MimeType URL::mimetype() const {

		String path{this->path()};

		const char *ptr = strrchr(path.c_str(),'.');
		if(ptr && *ptr) {
			return MimeTypeFactory(ptr+1);
		}

		return MimeType::none;

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

	bool URL::local() const {

		ParsedUri uri{*this};
		String scheme{uri.scheme.first, (size_t) (uri.scheme.afterLast - uri.scheme.first)};

		if(scheme.empty() || strcasecmp(scheme.c_str(),"file") == 0) {
			return true;
		}
	
		return false;
	}

	String URL::call(const HTTP::Method method, const char *payload) const {
		stringstream str;
		auto hdr = handler();
		int rc = hdr->perform(
			method, 
			payload, 
			[&str](uint64_t, uint64_t, const char *data, size_t){
				str << data;
				return false;
			}
		);
		hdr->except(rc);
		return String{str.str()};		
	}

 }

