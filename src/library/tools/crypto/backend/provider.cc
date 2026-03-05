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

 #define LOG_DOMAIN "sslkey"

 #include <config.h>

 #ifdef HAVE_OPENSSL_PROVIDER
 #include <private/crypto.h>
 #include <udjat/tools/file/text.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file/temporary.h>
 #include <cstdio>

 #ifdef HAVE_UNISTD_H
 #include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <openssl/provider.h>
 #include <openssl/bio.h>
 #include <openssl/evp.h>
 #include <openssl/pem.h>
 #include <udjat/tools/crypto.h>
 #include <udjat/tools/memory.h>
 #include <memory>
 #include <openssl/store.h>

 using namespace std;

 namespace Udjat{

	class UDJAT_PRIVATE SSLProvider : public Crypto::BackEnd {
	private:
		OSSL_PROVIDER *provider = nullptr;

	public:
		SSLProvider();
		~SSLProvider() override;
		void generate(const char *filename, const char *password, size_t mbits) override;

		void * encrypt(const void *data, size_t size, size_t &outsize) override;
		void * decrypt(const void *data, size_t size, size_t &outsize) override;
		void * digest(const void *data, size_t size, unsigned int &outsize) override;
		void * sign(const void *data, size_t size, size_t &outsize) override;
		bool verify(const void *sig, size_t siglen, const void *tbs, size_t tbslen) override;

	};
	
	std::shared_ptr<Crypto::BackEnd> Crypto::BackEnd::ProviderFactory() {
		return make_shared<SSLProvider>();
	}

	SSLProvider::SSLProvider() : Crypto::BackEnd{"provider","tpm2"} {

		class UDJAT_PRIVATE DefaultSSLProvider {
		private:
			OSSL_PROVIDER *provider = nullptr;

		public:

			DefaultSSLProvider() : provider{OSSL_PROVIDER_load(NULL, "default")}{
				if(!provider) {
					throw runtime_error("Could not load OpenSSL default provider");
				}
				Logger::String{"Loaded OpenSSL default provider"}.trace();
			}

			~DefaultSSLProvider() {
				OSSL_PROVIDER_unload(provider);
				Logger::String{"Unloaded OpenSSL default provider"}.trace();
			}

		};

		static DefaultSSLProvider default_provider;

		provider = OSSL_PROVIDER_load(NULL, type.c_str());
		if(!provider) {
			throw runtime_error(String{"Could not load OpenSSL provider '",type.c_str(),"'"});
		}
		Logger::String{"Using OpenSSL provider '",type.c_str(),"' for private key"}.trace();
	}

	SSLProvider::~SSLProvider() {
		OSSL_PROVIDER_unload(provider);
	}

	void SSLProvider::generate(const char *filename, const char *password, size_t mbits) {
		// https://github.com/tpm2-software/tpm2-openssl/blob/master/test/ec_genpkey_store_load.c
		// https://github.com/tpm2-software/tpm2-openssl/blob/master/test/rsa_genpkey_decrypt.c
		pkey = EVP_PKEY_Q_keygen(NULL, String{"provider=",type.c_str()}.c_str(), "RSA", mbits);
		if(!pkey) {
			throw Crypto::Exception("EVP_PKEY_Q_keygen failed");
		}
		if(filename && *filename) {
			save_private(filename, password);
		}
	}

	void * SSLProvider::encrypt(const void *data, size_t size, size_t &outsize) {
		throw system_error(ENOTSUP,system_category(),"Operation is not supported by the provider backend");
	}

	void * SSLProvider::decrypt(const void *data, size_t size, size_t &outsize) {
		throw system_error(ENOTSUP,system_category(),"Operation is not supported by the provider backend");
	}

	void * SSLProvider::digest(const void *data, size_t size, unsigned int &outsize) {
		throw system_error(ENOTSUP,system_category(),"Operation is not supported by the provider backend");
	}

	void * SSLProvider::sign(const void *data, size_t size, size_t &outsize) {
		throw system_error(ENOTSUP,system_category(),"Operation is not supported by the provider backend");
	}

	bool SSLProvider::verify(const void *sig, size_t siglen, const void *tbs, size_t tbslen) {
		throw system_error(ENOTSUP,system_category(),"Operation is not supported by the provider backend");
	}

 } 
 #endif // HAVE_OPENSSL_PROVIDER
 
