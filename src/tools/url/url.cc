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
 #include <private/protocol.h>
 #include <udjat/tools/http/client.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/url.h>
 #include <cstring>

#ifndef _WIN32
	#include <netdb.h>
#endif // _WIN32

 using namespace std;

 namespace Udjat {

	URL::Scheme URL::scheme() const {

		if(empty()) {
			throw runtime_error("Can't get scheme on empty URL");
		}

		size_t pos = find("://");
		if(pos == string::npos) {
			throw runtime_error(string{"Can't decode URL scheme on '"} + c_str() + "'");
		}

		return URL::Scheme{string::c_str(),pos};

	}

	URL::Components URL::ComponentsFactory() const {

		URL::Components components;

		const char *ptr;	// Temp pointer.
		size_t from, to;

		// Get query
		ptr = strchr(c_str(),'?');
		if(ptr) {
			components.query = (ptr+1);
		}

		ptr = c_str();
		if(ptr[0] == '/' || ptr[0] == '.') {
			// Filename, just extract path.
			components.scheme = "file";
			components.path = ptr;
			return components;
		}

		// Get URL Scheme.
		from = find("://");
		if(from == string::npos) {
			throw runtime_error(string{"Can't decode URL scheme on '"} + c_str() + "'");
		}
		from += 3;

		string scheme{string::c_str(),from-3};
		ptr = strrchr(scheme.c_str(),'+');
		if(ptr) {
			components.scheme.assign(ptr+1);
		} else {
			components.scheme.assign(scheme);
		}

		// Get hostname.
		string hostname;
		to = find("/",from);
		if(to == string::npos) {
			hostname.assign(string::c_str()+from);
		} else {
			hostname.assign(string::c_str()+from,to-from);
		}

		ptr = strrchr(hostname.c_str(),':');
		if(ptr) {
			components.hostname.assign(hostname.c_str(),(size_t) (ptr - hostname.c_str()));
			components.srvcname = (ptr+1);
		} else {
			components.hostname = hostname;
			components.srvcname.assign(components.scheme);
		}

		if(to == string::npos) {
			return components;
		}

		from = to;
		to = find("?",from);
		if(to == string::npos) {
			components.path.assign(string::c_str()+from);
			return components;
		}

		components.path.assign(string::c_str()+from,to-from);

		return components;

	}

	bool URL::local() const {
		const char *str = c_str();
		return str[0] == '/' || str[0] == '.' || strncasecmp(str,"file://",7) == 0;
	}

	int URL::test(const HTTP::Method method, const char *payload) const noexcept {

		if(empty()) {
			return ENODATA;
		}

		try {

			auto worker = Protocol::WorkerFactory(c_str());
			worker->method(method);
			worker->payload(payload);
			return worker->url(*this).test();

		} catch(const std::system_error &e) {

			cerr << "url\tError '" << e.what() << "' testing " << *this << endl;
			return (int) e.code().value();

		} catch(const std::exception &e) {

			cerr << "url\tError '" << e.what() << "' testing " << *this << endl;

		} catch(...) {

			cerr << "url\tUnexpected error testing " << *this << endl;

		}

		return -1;
	}

	std::string URL::get() const {
		return HTTP::Client(*this).get();
	}

	bool URL::get(Udjat::Value &value) const {
		Scheme scheme = this->scheme();
		return Protocol::for_each([this,&scheme,&value](const Protocol &protocol){
			if(protocol == scheme.c_str()) {
				return protocol.call(*this,value);
			}
			return false;
		});
	}

	std::string URL::get(const std::function<bool(uint64_t current, uint64_t total)> &progress) const {
		return HTTP::Client(*this).get(progress);
	}

	bool URL::get(const char *filename, const std::function<bool(uint64_t current, uint64_t total)> &progress) const {
		return HTTP::Client(*this).save(filename,progress);
	}

	void URL::get(const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer) {

		auto worker = Protocol::WorkerFactory(c_str());
		worker->method(HTTP::Get);
		worker->save(writer);

	}

	bool URL::get(const char *filename) const {
		return HTTP::Client(*this).save(filename);
	}

	std::string URL::post(const char *payload) const {
		return HTTP::Client(*this).post(payload);
	}

	std::string URL::filename(const std::function<bool(double current, double total)> &progress) {
		return HTTP::Client(*this).filename(progress);
	}

	std::string URL::filename() {
		return HTTP::Client(*this).filename();
	}

	String URL::argument(const char *name) const {

		String value;

		if(name && *name) {

			size_t szName = strlen(name);

			ComponentsFactory().query.for_each("&",[&value,szName,name](const String &v){

				if(v.size() > szName) {

					if(v[szName] == '=' && strncasecmp(v.c_str(),name,szName) == 0) {
						value = v.c_str()+szName+1;
						value.strip();
						return true;
					}
				}

				return false;
			});

		}

		return value;
	}


	int URL::Components::portnumber() const {

		for(const char *ptr = srvcname.c_str(); *ptr; ptr++) {
			if(!isdigit(*ptr)) {
				struct servent *se = getservbyname(srvcname.c_str(),NULL);
				if(se) {
					return htons(se->s_port);
				}
				throw system_error(EINVAL,system_category(),string{"The service '"} + srvcname.c_str() + "' is invalid");
			}
		}

		return stoi(srvcname);
	}

	URL URL::operator + (const char *path) {

		URL url{this->c_str()};
		url += path;
		return url;

	}

	URL & URL::operator += (const char *path) {

		// TODO: Extract arguments after '?' and rejoin after merge.

		while(!strncmp(path,"../",3)) {

			auto pos = rfind('/');
			if(pos == string::npos) {
				throw system_error(EINVAL,system_category(),"Cant merge path on URL");
			}

			resize(pos);
			path += 3;

		}

		if(!strncmp(path,"./",2)) {
			path++;
		}

		if(path[0] != '/') {
			append("/");
		}

		append(path);

		return *this;
	}

 }

 namespace std {

 	string to_string(const Udjat::URL &url) {
		return string(url);
 	}

 }
