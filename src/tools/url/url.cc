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
 #include <cstring>

 using namespace std;

 namespace Udjat {

	URL::Components URL::ComponentsFactory() const {

		URL::Components components;

#ifdef _WIN32

		#error Implement using URL_COMPONENTS from winhttp

#else
		const char *ptr;	// Temp pointer.

		size_t from, to;

		// Get URL Scheme.
		from = find("://");
		if(from == string::npos) {
			throw runtime_error(string{"Can't decode URL scheme on '"} + c_str() + "'");
		}
		from += 3;

		string scheme{c_str(),from-3};
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
			hostname.assign(c_str()+from);
		} else {
			hostname.assign(c_str()+from,to-from);
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
			components.path.assign(c_str()+from);
			return components;
		}

		components.path.assign(c_str()+from,to-from);
		components.query = c_str()+to+1;

#endif // _WIN32

		return components;

	}


	/*
	URL::URL() {
	}

	URL::URL(const char *url) : URL() {
		assign(url);
	}

	URL::~URL() {
	}

	const char * URL::getPortName() const {
		if(port.empty()) {
			const char *portname = protocol->getDefaultPortName();
			if(!(portname && *portname)) {
				portname = protocol->c_str();
			}
			return portname;
		}
		return port.c_str();
	}

	int URL::getPortNumber() const {

		const char * portname = getPortName();

		// Convert numeric ports direct.
		int port = atoi(portname);
		if(port)
			return port;

		// Can't convert numeric, search by port name.
		struct servent *se = getservbyname(portname,NULL);
		if(!se) {
			throw runtime_error(string{"Can't find port number for service '"} + portname + "'");
		}

		return htons(se->s_port);

	}

	const char * URL::getFileName() const {
		return filename.c_str();
	}

	URL & URL::assign(const char *u) {

		string url = unescape(u);

		if(url.empty()) {
			throw runtime_error("URL value can't be empty");
		}

		size_t from, to;

		// Get scheme and find associated protocol manager.
		from = url.find("://");
		if(from == string::npos) {
			throw runtime_error(string{"Can't decode URL scheme on '"} + url + "'");
		}
		from += 3;

		this->protocol = Controller::getInstance().find(string(url.c_str(),from-3).c_str());

		// Get hostname and port.
		string domain;

		to = url.find("/",from);
		if(to == string::npos) {
			domain.assign(url.c_str()+from);
		} else {
			domain.assign(url.c_str()+from,to-from);
		}
		from = to;

		to = domain.find(':');
		if(to == string::npos) {
			this->domain.assign(domain);
		} else {
			this->domain.assign(domain.c_str(),to);
			this->port.assign(domain.c_str()+to+1);
		}

		if(from ==string::npos)
			return *this;

		to = url.find("?",from);

		if(to == string::npos) {
			this->filename.assign(url.c_str()+from);
			return *this;
		}

		this->filename.assign(url.c_str()+from,to-from);

		// TODO: Parse arguments.

		return *this;
	}

	std::string URL::to_string() const {

		string rc{protocol->c_str()};

		rc += "://";
		rc += domain;

		if(!port.empty()) {
			rc += ":";
			rc += port;
		}

		if(!filename.empty()) {
			rc += "/";
			rc += filename;
		}

		return rc;
	}
	*/

 }

