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

 #define LOG_DOMAIN "ssl"

 #include <config.h>

 #include <openssl/err.h>
 #include <openssl/x509.h>
 #include <openssl/bio.h>
 #include <openssl/pem.h>
 #include <string>
 #include <libgen.h>
 #include <udjat/tools/crypto.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/logger.h>

 using namespace Udjat;
 using namespace std;

 namespace Udjat {

	static int error_cb(const char *str, size_t len, void *u) {
		string line{str,len};
		Logger::String{line.c_str()}.error();
		return 1;
	}

	Udjat::Crypto::Exception::Exception(const char *msg) : Udjat::Exception{ msg } {
		Logger::String{msg}.error("ssl");
		ERR_print_errors_cb(error_cb,&info.body);
	}

 }

