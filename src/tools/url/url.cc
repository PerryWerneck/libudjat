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

 #include "private.h"
 #include <udjat/tools/http/client.h>
 #include <cstring>

#ifndef _WIN32
	#include <netdb.h>
#endif // _WIN32

 using namespace std;

 namespace Udjat {

	std::string URL::scheme() const {

		size_t pos = find("://");
		if(pos == string::npos) {
			throw runtime_error(string{"Can't decode URL scheme on '"} + c_str() + "'");
		}

		return string{string::c_str(),pos};

	}

	URL::Components URL::ComponentsFactory() const {

		URL::Components components;

		const char *ptr;	// Temp pointer.

		size_t from, to;

		// Get URL Scheme.
		from = find("://");
		if(from == string::npos) {
			throw runtime_error(string{"Can't decode URL scheme on '"} + c_str() + "'");
		}
		from += 3;

		string scheme{string::c_str(),from-3};
		ptr = strrchr(scheme.c_str(),'+');
		if(ptr) {
			components.scheme = (ptr+1);
		} else {
			components.scheme = scheme;
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
			components.srvcname = components.scheme;
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
		components.query = string::c_str()+to+1;

		return components;

	}

	unsigned short URL::test() const noexcept {

		try {

			const Protocol * protocol = Protocol::find(*this);
			if(!protocol) {
				cerr << "url\tCant find a protocol handler for " << *this << endl;
				return -EINVAL;
			}

			auto worker = protocol->WorkerFactory();
			if(worker) {
				worker->url(*this);
			}

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

	std::string URL::get(const std::function<bool(uint64_t current, uint64_t total)> &progress) const {
		return HTTP::Client(*this).get(progress);
	}

	bool URL::get(const char *filename, const std::function<bool(uint64_t current, uint64_t total)> &progress) const {
		return HTTP::Client(*this).save(filename,progress);
	}

	bool URL::get(const char *filename) const {
		return HTTP::Client(*this).save(filename);
	}

	std::string URL::post(const char *payload) const {
		return HTTP::Client(*this).post(payload);
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

 }

 namespace std {

 	string to_string(const Udjat::URL &url) {
		return string(url);
 	}

 }
