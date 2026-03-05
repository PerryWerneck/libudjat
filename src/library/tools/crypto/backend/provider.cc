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
		EVP_PKEY *pubkey = nullptr;

		/*
		/// @brief Create a new context for the provider, check sanity of the key first.
		/// @return A shared pointer to the context.
		/// @throw Crypto::Exception If the context could not be created.
		/// @throw std::logic_error If the key is not an RSA key or has no modulus/bits.	
		std::shared_ptr<EVP_PKEY_CTX> make_context() {

			if (EVP_PKEY_get_id(pkey) != EVP_PKEY_RSA) {
				throw logic_error("Not an RSA key");
			}

			if (EVP_PKEY_get_bits(pkey) <= 0) {
				throw logic_error("Key has no modulus/bits (uninitialized key)");
			}		

			if(!pubkey) {
				// 1. Get the public key bits from the TPM-backed pkey
				unsigned char* pub_buf = nullptr;
				int pub_len = i2d_PUBKEY(pkey, &pub_buf); // Export to DER format

				if (pub_len <= 0) {
					throw Crypto::Exception("Failed to export TPM public key");
				}

				// 2. Re-import into a "clean" EVP_PKEY (owned by the Default Provider)
				const unsigned char* p = pub_buf;
				pubkey = d2i_PUBKEY(nullptr, &p, pub_len);
				OPENSSL_free(pub_buf);

				if (!pubkey) {
					throw Crypto::Exception("Failed to re-import public key");
				}
			}

			if (!EVP_PKEY_get0_RSA(pkey)) {
				throw logic_error("Internal RSA structure is missing!");
			}		

			auto ctx = make_handle(
				EVP_PKEY_CTX_new_from_pkey(NULL, pkey, String{"provider=",type.c_str()}.c_str()), 
				EVP_PKEY_CTX_free
			);

			if(!ctx) {
				throw Crypto::Exception("EVP_PKEY_CTX_new_from_pkey failed");
			}

			return ctx;
		}
		*/

		void sanity_check() {

			if (EVP_PKEY_get_id(pkey) != EVP_PKEY_RSA) {
				throw logic_error("Not an RSA key");
			}

			if (EVP_PKEY_get_bits(pkey) <= 0) {
				throw logic_error("Key has no modulus/bits (uninitialized key)");
			}		

		}

		/// @brief Get context for private key operations.
		/// @return A shared pointer to the private key context.
		std::shared_ptr<EVP_PKEY_CTX> get_private_key_context() override;	

		/// @brief Get context for public key operations.
		/// @return A shared pointer to the public key context.
		std::shared_ptr<EVP_PKEY_CTX> get_public_key_context() override;	

	public:
		SSLProvider();
		~SSLProvider() override;
		void generate(const char *filename, const char *password, size_t mbits) override;

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
		if(pubkey) {
			debug("Unloading public key");
			EVP_PKEY_free(pubkey);
			pubkey = NULL;
		}
		OSSL_PROVIDER_unload(provider);
	}

	std::shared_ptr<EVP_PKEY_CTX> SSLProvider::get_private_key_context() {

		auto ctx = make_handle(
			EVP_PKEY_CTX_new_from_pkey(NULL, pkey, String{"provider=",type.c_str()}.c_str()), 
			EVP_PKEY_CTX_free
		);

		if(!ctx) {
			throw Crypto::Exception("EVP_PKEY_CTX_new_from_pkey failed");
		}

		return ctx;

	}

	std::shared_ptr<EVP_PKEY_CTX> SSLProvider::get_public_key_context() {

		if(!pubkey) {

			debug("Getting pubkey from TPM")
			sanity_check();

			// 1. Get the public key bits from the TPM-backed pkey
			unsigned char* pub_buf = nullptr;
			int pub_len = i2d_PUBKEY(pkey, &pub_buf); // Export to DER format

			if (pub_len <= 0) {
				throw Crypto::Exception("Failed to export TPM public key");
			}

			// 2. Re-import into a "clean" EVP_PKEY (owned by the Default Provider)
			const unsigned char* p = pub_buf;
			pubkey = d2i_PUBKEY(nullptr, &p, pub_len);
			OPENSSL_free(pub_buf);

			if (!pubkey) {
				throw Crypto::Exception("Failed to re-import public key");
			}
		}

		auto ctx = make_handle(
			EVP_PKEY_CTX_new_from_pkey(NULL, pubkey, String{"provider=",type.c_str()}.c_str()), 
			EVP_PKEY_CTX_free
		);

		if(!ctx) {
			throw Crypto::Exception("EVP_PKEY_CTX_new_from_pkey failed");
		}

		return ctx;
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

	/*
	void * SSLProvider::encrypt(const void *data, size_t size, size_t &outsize) {

		// Reference: https://linux.die.net/man/3/evp_pkey_encrypt

		debug("Using provider encript()");
		debug("keysize=", EVP_PKEY_get_size(pkey), " bits=", EVP_PKEY_get_bits(pkey), " data size=", size);

		auto ctx = make_handle(EVP_PKEY_CTX_new(get_pubkey(), NULL), EVP_PKEY_CTX_free);
		if(!ctx) {
			throw Crypto::Exception("EVP_PKEY_CTX_new failed");
		}

		if(EVP_PKEY_encrypt_init(ctx.get()) <= 0) {
			throw Crypto::Exception("EVP_PKEY_encrypt_init failed");
		}

		if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) {
			throw Crypto::Exception("EVP_PKEY_CTX_set_rsa_padding failed");
		}
			
		if(EVP_PKEY_encrypt(ctx.get(), NULL, &outsize, (const unsigned char *) data, size) <= 0) {
			throw Crypto::Exception("EVP_PKEY_encrypt failed");
		}

		auto out = malloc(outsize + 1);
		if(!out) {
			throw runtime_error("malloc failed");
		}

		if(EVP_PKEY_encrypt(ctx.get(), (unsigned char *) out, &outsize, (const unsigned char *) data, size) <= 0) {
			free(out);
			throw Crypto::Exception("EVP_PKEY_encrypt failed");
		}

		((uint8_t *) out)[outsize] = 0;
		return out;
	}
	*/

	/*
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
	*/

 } 
 #endif // HAVE_OPENSSL_PROVIDER
 
