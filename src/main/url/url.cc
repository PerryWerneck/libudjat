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

 namespace Udjat {

	URL::URL() {
	}

	URL::URL(const char *url) : URL() {
		assign(url);
	}

	void URL::insert(std::shared_ptr<Protocol> protocol) {
		Controller::getInstance().insert(protocol);
	}

	URL::~URL() {
	}

	void URL::getInfo(Udjat::Response &response) {
		Controller::getInstance().getInfo(response);
	}

	URL & URL::assign(const char *u) {

		string url = unescape(u);
		size_t from, to;

		// Get scheme and find associated protocol manager.
		from = url.find("://");
		if(from == string::npos) {
			throw runtime_error("Can't decode URL scheme");
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

	/// @brief Connect to host.
	/// @return Socket connected to host.
	int URL::connect(time_t timeout) const {
		return protocol->connect(*this,timeout);
	}

	static int xdigit_value(const char scanner) {

		if(scanner >= '0' && scanner <= '9') {
			return scanner - '0';
		}

		if(scanner >= 'A' && scanner <= 'F') {
			return 10 + (scanner - 'A');
		}

		if(scanner >= 'a' && scanner <= 'f') {
			return 10 + (scanner - 'a');
		}

		throw runtime_error("Invalid escape character");
	}

	static int unescape_character(const char *scanner) {

		int first_digit = xdigit_value(*scanner++);
		int second_digit = xdigit_value(*scanner++);

		return (first_digit << 4) | second_digit;

	}

	std::string URL::unescape(const char *src) {

		char 		* buffer;
		char		* dst;

		buffer = dst = new char[strlen(src)+1];

		try {

			while(*src) {

				if(*src == '%') {

					if(src[1] && src[2]) {

						*(dst++) = (char) unescape_character(++src);
						src += 2;

					} else {

						throw runtime_error("Unexpected escape sequence");

					}

				} else {

					*(dst++) = *(src++);

				}

			}

			*dst = 0;

		} catch(...) {

			delete[] buffer;
			throw;

		}

		string rc = string{buffer};
		delete[] buffer;

		return rc;

	}

	std::shared_ptr<URL::Response> URL::get(const char *mimetype) {
		return protocol->call(*this,URL::Method::Get,mimetype);
	}

	std::shared_ptr<URL::Response> URL::post(const char *payload, const char *mimetype) {
		return protocol->call(*this,URL::Method::Post,mimetype,payload);
	}

	std::string URL::to_string() const {

		string rc{protocol->c_str()};

		if(!port.empty()) {
			rc += ":";
			rc += port;
		}

		rc += domain;

		if(!filename.empty()) {
			rc += "/";
			rc += filename;
		}

		return rc;
	}

 }

