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
 #include <udjat/tools/url/handler.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/ui/console.h>
 #include <uriparser/Uri.h>
 #include <stdexcept>
 #include <string>
 #include <list>
 #include <memory>
 #include <sstream>
 #include <private/urlparser.h>
 #include <libgen.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/base64.h>
 #include <algorithm>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifndef _WIN32
	#include <netdb.h>
 	#include <cstdio>
 #endif // _WIN32

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

	static void sanitize(std::string &path) {
		for(const char *filter : { "//", "\\\\"}) {
			size_t pos;
			while((pos = path.find(filter)) != string::npos) {
				char buffer[] = { filter[0], 0 };
				path.replace(pos,2,buffer);
			}
		}
	}

	String URL::path(bool strip) const {

		ParsedUri uri{*this};

		String result;

		if (uri.pathHead) {
			for(UriPathSegmentA *pathSegment = uri.pathHead; pathSegment; pathSegment = pathSegment->next) {
				if(!strip) {
					result += '/';
				}
				size_t len = (size_t) (pathSegment->text.afterLast - pathSegment->text.first);
				result += string{pathSegment->text.first,len};
			}
		}

		sanitize(result);

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
		if(ptr && *(ptr+1)) {
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
		{
			string new_path;
			for(const string &segment : segments) {
				new_path.append("/");
				new_path.append(segment);
			}
			
			if(*ptr == '/') {
				ptr++;
			}
			
			if(*ptr) {
				new_path.append("/");
				new_path.append(ptr);
			}

			sanitize(new_path);
			newUri.append(new_path);

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

	String URL::call(const HTTP::Method method, const char *payload, const bool console) const {
		stringstream str;
		auto hdr = handler();
		int rc = hdr->perform(
			method, 
			payload, 
			[this,&str,console](uint64_t current, uint64_t total, const void *data, size_t len) -> bool {
				if(data && len) {
					str.write((const char *) data,len);
				}
				if(console) {
					URL::progress_to_console(this->c_str(),current,total);
				}
				return false;
			}
		);
		hdr->except(rc);
		return String{str.str()};		
	}

	bool URL::get(Udjat::Value &value, const HTTP::Method method, const char *payload) const {
		return handler()->get(value,method,payload);
	}

	int URL::test(const HTTP::Method method, const char *payload) const {
		return handler()->test(method,payload);
	}

	String URL::get(const std::function<bool(uint64_t current, uint64_t total)> &progress) const {
		return handler()->get(HTTP::Get,"",progress);
	}

	String URL::post(const char *payload, const std::function<bool(uint64_t current, uint64_t total)> &progress) const {
		return handler()->get(HTTP::Post,payload,progress);
	}

	String URL::get(bool console) const {
		return get([&](uint64_t current, uint64_t total) {
			if(console) {
				URL::progress_to_console(this->c_str(),current,total);
			}
			return false;
		});
	}

	bool URL::get(const char *filename, const std::function<bool(uint64_t current, uint64_t total)> &progress) {
		return handler()->get(filename,HTTP::Get,"",progress);
	}

	bool URL::get(const char *filename,const HTTP::Method method, const char *payload) {
		return handler()->get(filename,method,payload);
	}

	int URL::get(const std::function<bool(uint64_t current, uint64_t total, const void *buf, size_t length)> &writer) {
		return handler()->perform(
			HTTP::Get,
			"",
			writer
		);	
	}

	std::string URL::cache(const std::function<bool(double current, double total)> &progress) {

		Application::CacheDir name{"urls"};
		name += Base64::encode(c_str());

		handler()->get(name.c_str(),HTTP::Get,"",progress);

		return name;
	}

	std::string URL::tempfile(const std::function<bool(double current, double total)> &progress) {

		string name = File::Temporary::create();

		try {
			handler()->get(name.c_str(),HTTP::Get,"",progress);
		} catch(...) {
			if(Logger::enabled(Logger::Level::Debug)) {
				Logger::String("Download failed, removing ",name.c_str()).write(Logger::Debug);
			}	
			unlink(name.c_str());
			throw;
		}

		return name;
	}


	std::string URL::cache() {
		return cache([](double,double) -> bool { return false; });
	}

	std::string URL::tempfile() {
		return cache([](double,double) -> bool { return false; });
	}

	bool URL::progress_to_console(const char *url, uint64_t current, uint64_t total) noexcept {
		return progress_to_console("",url,current,total);
	}

	bool URL::progress_to_console(const char *prefix, const char *url, uint64_t current, uint64_t total) noexcept {
		return UI::Console{}.progress(prefix,url,current,total);
	}

 }

