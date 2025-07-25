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
 #include <udjat/tools/string.h>
 #include <stdexcept>
 #include <cstring>

 using namespace std;

 namespace Udjat {

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

		throw runtime_error(string{"Escape character '"} + scanner + "' is invalid");
	}

	static int unescape_character(const char *scanner) {

		int first_digit = xdigit_value(*scanner++);
		int second_digit = xdigit_value(*scanner++);

		return (first_digit << 4) | second_digit;

	}

	/*
	URL & URL::unescape() {
		std::string::assign(unescape(c_str()));
		return *this;
	}
	*/

	Udjat::String & Udjat::String::escape(char marker) {

		size_t maxlen = size()*4;
		char * buffer = new char[maxlen+4];
		char * dst = buffer;
		const char *src = c_str();

		size_t index = 0;
		while(*src) {

			if(index >= maxlen) {
				delete[] buffer;
				throw std::logic_error("Escaped string is bigger than expected");
			}

			if(isalnum(*src) && *src != marker) {

				dst[index++] = *(src++);

			} else {
				dst[index++] = marker;

				char hexvalue[4];
				snprintf(hexvalue,3,"%02X",(unsigned int) *(src++));
				dst[index++] = hexvalue[0];
				dst[index++] = hexvalue[1];

			}

		}
		dst[index] = 0;
		assign(buffer);
		delete[] buffer;

		return *this;

	}

	Udjat::String & Udjat::String::unescape(char marker) {

		char * buffer = new char[size()+1];
		char * dst = buffer;
		const char *src = c_str();

		try {

			while(*src) {

				if(*src == marker) {

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

		assign(buffer);
		delete[] buffer;

		return *this;
	}

	/*
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
*/

 }
