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

 #define LOG_DOMAIN "pkey"

 #include <config.h>
 #include <udjat/tools/file/text.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <cstdio>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifdef ENABLE_OPENSSL_ENGINES
 #define OPENSSL_SUPPRESS_DEPRECATED
 #include <openssl/engine.h>
 #endif // ENABLE_OPENSSL_ENGINES

 #ifdef ENABLE_OPENSSL_PROVIDER
 #include <openssl/provider.h>
 #endif // ENABLE_OPENSSL_PROVIDER

 #include <openssl/bio.h>

 #include <openssl/evp.h>
 #include <udjat/tools/ssl.h>
 #include <memory>

 using namespace std;

 using BIO_PTR = std::unique_ptr<BIO, decltype(&BIO_free_all)>;
 using CTX_PTR = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
 using BIGNUM_PTR = std::unique_ptr<BIGNUM, decltype(&BN_free)>;

 namespace Udjat {

	class UDJAT_PRIVATE SSL::BackEnd {
	public:
		constexpr BackEnd() {
		}

		virtual ~BackEnd() {
		}

		virtual EVP_PKEY * load(const File::Text &file) {

			auto bio = BIO_PTR(BIO_new_mem_buf((void*)file.c_str(), -1),BIO_free_all);
			if(!bio) {
				throw SSL::Exception("BIO_new_mem_buf");
			}

			// TODO: Implement callback for password.
			return PEM_read_bio_PrivateKey(bio.get(), NULL, NULL, NULL);

		}

		virtual EVP_PKEY * generate(size_t mbits = 2048) = 0;
	};

	static std::shared_ptr<SSL::BackEnd> BackEndFactory(const Udjat::String &name) {

		switch(name.select("legacy","engine","provider",NULL)) {
		case 0: // Legacy
			class LegacyBackEnd : public SSL::BackEnd {
			public:
				LegacyBackEnd() {
				};

				EVP_PKEY * generate(size_t mbits) override {
					return EVP_RSA_gen(mbits);
				}

			};

			return make_shared<LegacyBackEnd>();

		case 1: // engine
#ifdef ENABLE_OPENSSL_ENGINES
			class EngineBackEnd : public SSL::BackEnd {
			public:
				ENGINE *engine;

				EngineBackEnd() {
					ENGINE_load_builtin_engines();
					engine = ENGINE_by_id(Config::Value<string>("ssl","tpm-engine","tpm2tss").c_str());
					if(!engine) {
						throw runtime_error("Could not find the TPM engine");
					}

					if(ENGINE_init(engine) == 0) {
						throw runtime_error("Could not initialize the TPM engine");
					}
				}

				~EngineBackEnd() override {
					ENGINE_finish(engine);
				}

				EVP_PKEY * generate(size_t mbits) override {

					auto ctx = CTX_PTR(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, engine),EVP_PKEY_CTX_free);
					if(!ctx) {
						throw SSL::Exception("EVP_PKEY_CTX_new_from_name");
					}

					if(EVP_PKEY_keygen_init(ctx.get()) <= 0) {
						throw SSL::Exception("EVP_PKEY_keygen_init");
					}
		
					if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), mbits) <= 0) {
						throw SSL::Exception("EVP_PKEY_CTX_set_rsa_keygen_bits");
					}

					auto bn = BIGNUM_PTR(BN_new(),BN_free);
					if (!bn) {
						throw runtime_error("Error creating BIGNUM for public exponent.");
					}

					if(BN_set_word(bn.get(), RSA_F4) <= 0) {
						throw runtime_error("Error setting public exponent.");
					}
					
					if(EVP_PKEY_CTX_set_rsa_keygen_pubexp(ctx.get(), bn.get()) <= 0) {
						throw runtime_error("Error setting RSA keygen public exponent.");
					}					

					EVP_PKEY *pkey;
					if(EVP_PKEY_keygen(ctx.get(), &pkey) <= 0) {
						throw SSL::Exception("EVP_PKEY_keygen");
					}

					return pkey;
						
				}

			};

			return make_shared<EngineBackEnd>();

#else
			throw runtime_error("Unable to use OpenSSL engine");
			break;
#endif // ENABLE_OPENSSL_ENGINES

		case 2: // provider
#ifdef ENABLE_OPENSSL_PROVIDER
			class ProviderBackEnd : public SSL::BackEnd {
			public:
				OSSL_PROVIDER *provider;

				ProviderBackEnd() {
					provider = OSSL_PROVIDER_load(NULL, Config::Value<string>("ssl","provider-engine","tpm2").c_str());
				}

				~ProviderBackEnd() override {
					OSSL_PROVIDER_unload(provider);
				}

				EVP_PKEY * generate(size_t mbits) override {
					return EVP_PKEY_Q_keygen(NULL, "provider=tpm2", "RSA", mbits);
				}

			};

			return make_shared<ProviderBackEnd>();
#else
			throw runtime_error("Unable to use OpenSSL providers");
			break;
#endif // ENABLE_OPENSSL_PROVIDER

		}

		throw SSL::Exception(String{"Unable to activate SSL backend '",name.c_str(),"'"});

	}

	SSL::Key::Private::Private(const char *filename, const char *password, bool autogenerate) {

		if(filename && *filename && access(filename,R_OK) == 0) {
			// File exists, load it.
			load(filename,password);	
			return;
		}

		if(autogenerate) {

			String mode{Config::Value<string>{"ssl","genkey","auto"}.c_str()};

			if(strcasecmp(mode.c_str(),"auto") == 0) {
				// Check if tpm is available

			}

			backend = BackEndFactory(mode);
			pkey = backend->generate((size_t) Config::Value<unsigned int>{"ssl","mbits",2048});
			if(!pkey) {
				throw SSL::Exception("Error generating private key");
			}

		}

		if(filename && *filename) {
			save(filename,password);
		}

	}

	SSL::Key::Private::~Private() {

		if(pkey) {
			EVP_PKEY_free(pkey);
			pkey = NULL;
		}

	}

	void SSL::Key::Private::load(const char *filename, const char *passwd) {

        static const char *tpm2_tags[] = {
                "-----BEGIN TSS2 PRIVATE KEY-----",
                "-----BEGIN TSS2 KEY BLOB-----",
                NULL
        };

		if(pkey) {
			EVP_PKEY_free(pkey);
			pkey = NULL;
		}

		File::Text text{filename};
		String mode{"legacy"};

		for(const char *tpm_tag : tpm2_tags) {
			if(strstr(text.c_str(),tpm_tag)) {
				Logger::String{"Found TPM2 key on ",filename}.trace();
				mode = Config::Value<string>{"ssl","tpm-mode","engine"}.c_str();
				break;
			}
		}

		backend = BackEndFactory(mode);
		pkey = backend->load(text);

	}

	void SSL::Key::Private::save(const char *filename, const char *password) {

		FILE *priv_key_file = fopen(filename, "w");
		if(!priv_key_file) {
			throw system_error(errno,system_category(),filename);
		}

		if(PEM_write_PrivateKey(
			priv_key_file, 
			pkey, 
			EVP_des_ede3_cbc(), 
			(unsigned char *) password, 
			(password ? strlen(password) : 0), 
			NULL, 
			NULL
		) != 1) {
			fclose(priv_key_file);
			throw SSL::Exception("PEM_write_PrivateKey failed");
		}

		fclose(priv_key_file);

	}

 }

