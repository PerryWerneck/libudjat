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

 #define LOG_DOMAIN "crypto"

 #include <config.h>
 #include <private/crypto.h>
 #include <udjat/tools/file/text.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file/temporary.h>
 #include <cstdio>

 #ifdef HAVE_UNISTD_H
 #include <unistd.h>
 #endif // HAVE_UNISTD_H

 #define OPENSSL_SUPPRESS_DEPRECATED
 

 #include <openssl/bio.h>
 #include <openssl/evp.h>
 #include <openssl/rsa.h> 
 #include <openssl/pem.h>
 #include <udjat/tools/crypto.h>
 #include <memory>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Crypto::BackEnd> Crypto::BackEnd::LegacyFactory() {

		/// @brief Legacy backend for OpenSSL.
		/// This backend uses the legacy OpenSSL API to generate and manage keys.
		class Legacy : public Crypto::BackEnd {
		public:
			Legacy() : BackEnd{"legacy","legacy"} {
				Logger::String{"Using legacy OpenSSL backend for private key"}.trace();
			};

			void generate(const char *filename, const char *password, size_t mbits) override {
				pkey = EVP_RSA_gen(mbits);
				if(!pkey) {
					throw Crypto::Exception("EVP_RSA_gen failed");
				} else if(filename && *filename) {
					save_private(filename, password);
				}
			}

		};
		
		return make_shared<Legacy>();

	}

 }
