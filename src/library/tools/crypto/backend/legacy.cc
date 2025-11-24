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
 #include <openssl/opensslv.h>
 #include <udjat/tools/crypto.h>
 #include <udjat/tools/memory.h>
 #include <memory>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Crypto::BackEnd> Crypto::BackEnd::LegacyFactory() {

		/// @brief Legacy backend for OpenSSL.
		/// This backend uses the legacy OpenSSL API to generate and manage keys.
		class Legacy : public Crypto::BackEnd {
		public:
			Legacy() : BackEnd{"legacy","legacy"} {
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
				Logger::String{"Using legacy OpenSSL V3 backend for private key"}.trace();
#else
				Logger::String{"Using legacy OpenSSL backend for private key"}.trace();
#endif
			};

			void generate(const char *filename, const char *password, size_t mbits) override {

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
				pkey = EVP_RSA_gen(mbits);
				if(!pkey) {
					throw Crypto::Exception("EVP_RSA_gen failed");
				}  
#else
				auto rsa = make_handle(RSA_new(),RSA_free)	;
				if(!rsa.get()) {
					throw Crypto::Exception("RSA_new failed");
				}	

				auto bignum = make_handle(BN_new,BN_free);
				if(!bignum.get()) {
					throw Crypto::Exception("BN_new failed");
				}
				if(BN_set_word(bignum.get(), RSA_F4) != 1) {
					throw Crypto::Exception("BN_set_word failed");
				}

				if(RSA_generate_key_ex(rsa.get(), mbits, bignum.get(), NULL) != 1) {
					throw Crypto::Exception("RSA_generate_key_ex failed");
				}

				pkey = EVP_PKEY_new();
				if(!pkey) {
					throw Crypto::Exception("EVP_PKEY_new failed");
				}

				if(EVP_PKEY_assign_RSA(pkey, rsa.get()) != 1) {
					EVP_PKEY_free(pkey);
					throw Crypto::Exception("EVP_PKEY_assign_RSA failed");
				}

#endif // OPENSSL_VERSION_NUMBER
				
				if(filename && *filename) {
					save_private(filename, password);
				}
			}

		};
		
		return make_shared<Legacy>();

	}

 }
