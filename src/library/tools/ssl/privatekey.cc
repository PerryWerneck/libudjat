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
 #include <udjat/tools/file/temporary.h>
 #include <cstdio>

 #ifdef HAVE_UNISTD_H
 #include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifdef HAVE_OPENSSL_ENGINE
 #define OPENSSL_SUPPRESS_DEPRECATED
 #include <openssl/engine.h>
 #endif // HAVE_OPENSSL_ENGINE

 #ifdef HAVE_TPM2_TSS_ENGINE_H
 #include <tpm2-tss-engine.h>
 #endif // HAVE_TPM2_TSS_ENGINE_H

 #ifdef HAVE_OPENSSL_PROVIDER
 #include <openssl/provider.h>
 #endif // HAVE_OPENSSL_PROVIDER

 #include <openssl/bio.h>

 #include <openssl/evp.h>
 #include <udjat/tools/ssl.h>
 #include <memory>

 using namespace std;

 using BIO_PTR = std::unique_ptr<BIO, decltype(&BIO_free_all)>;
 using CTX_PTR = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
 using BIGNUM_PTR = std::unique_ptr<BIGNUM, decltype(&BN_free)>;
 using RSA_PTR = std::unique_ptr<RSA, decltype(&RSA_free)>;

 namespace Udjat {

	class UDJAT_PRIVATE SSL::BackEnd {
	public:
		constexpr BackEnd() {
		}

		virtual ~BackEnd() {
		}

		virtual void save(EVP_PKEY *pkey, const char *filename, const char *password) {

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

		virtual EVP_PKEY * load(const File::Text &file) {

			debug("Loading \n",file.c_str());

			auto bio = BIO_PTR(BIO_new_mem_buf((void*)file.c_str(), -1),BIO_free_all);
			if(!bio) {
				throw SSL::Exception("BIO_new_mem_buf");
			}

			// TODO: Implement callback for password.
			return PEM_read_bio_PrivateKey(bio.get(), NULL, NULL, NULL);

		}

		virtual std::string toString(EVP_PKEY *pkey, const char *) {

			BIO_PTR bio(BIO_new(BIO_s_mem()), BIO_free_all);
			if(!bio) {
				throw SSL::Exception("BIO_new failed");
			}

			if(PEM_write_bio_PrivateKey(bio.get(), pkey, NULL, NULL, 0, NULL, NULL) != 1) {
				throw SSL::Exception("PEM_write_bio_PrivateKey failed");
			}

			char *data;
			long len = BIO_get_mem_data(bio.get(), &data);
			return std::string(data, len);
		}
		
		virtual EVP_PKEY * generate(const char *filename, const char *password, size_t mbits = 2048) = 0;
	};

	static std::shared_ptr<SSL::BackEnd> BackEndFactory(Udjat::String name) {

		if(strcasecmp(name.c_str(),"auto") == 0) {

			// Check if tpm is available
			if(access("/dev/tpm0",F_OK) != 0) {
				name = "legacy";
			} else {
#if defined(HAVE_OPENSSL_ENGINE)
				// /usr/lib64/engines-3/tpm2.so
				name = Config::Value<string>{"ssl","tpm2-backend","engine"}.c_str();
#elif defined(HAVE_OPENSSL_PROVIDER)
				// /usr/lib64/ossl-modules/tpm2.so
				name = Config::Value<string>{"ssl","tpm2-backend","provider"}.c_str();
#else
				Logger::String{"No OpenSSL backend available, using legacy mode"}.warning();
				name = "legacy";
#endif
			}
		}

		switch(name.select("legacy","engine","provider",NULL)) {
		case 0: // Legacy
			class LegacyBackEnd : public SSL::BackEnd {
			public:
				LegacyBackEnd() {
					Logger::String{"Using legacy OpenSSL backend for private key"}.trace();
				};

				EVP_PKEY * generate(const char *filename, const char *password, size_t mbits) override {
					EVP_PKEY *key = EVP_RSA_gen(mbits);
					if(key && filename && *filename) {
						save(key, filename, password);
					} else if(!key) {
						throw SSL::Exception("EVP_RSA_gen failed");
					}
					return key;
				}

			};

			return make_shared<LegacyBackEnd>();

		case 1: // engine
#ifdef HAVE_OPENSSL_ENGINE

			// Requires package openssl_tpm2_engine

			class EngineBackEnd : public SSL::BackEnd {
			public:
				ENGINE *engine;

				EngineBackEnd() {
					ENGINE_load_builtin_engines();
					engine = ENGINE_by_id(Config::Value<string>("ssl","tpm-engine","tpm2tss").c_str());
					if(!engine) {
						throw runtime_error("Could not load OpenSSL engine");
					}

					if(ENGINE_init(engine) == 0) {
						throw runtime_error("Could not initialize OpenSSL engine");
					}

					Logger::String{"Using OpenSSL engine backend for private key"}.trace();
				}

				~EngineBackEnd() override {
					ENGINE_finish(engine);
				}

#if defined(HAVE_TPM2_TSS_ENGINE_H)
				std::string toString(EVP_PKEY *, const char *filename) override {
					if(!(filename && *filename)) {
						throw runtime_error("Filename is required to extract key from TPM2TSS engine.");
					}
					return File::Text{filename}.c_str();
				}
#endif // HAVE_TPM2_TSS_ENGINE_H

				EVP_PKEY * generate(const char *filename, const char *, size_t mbits) override {

#if defined(HAVE_TPM2_TSS_ENGINE_H)

					if(!(filename && *filename)) {
						throw runtime_error("Filename is required for TPM2TSS engine key generation.");
					}

					// Reference: https://github.com/tpm2-software/tpm2-tss-engine/blob/master/src/tpm2tss-genkey.c
					auto bignum = BIGNUM_PTR(BN_new(),BN_free);
					if (!bignum) {
						throw runtime_error("Error creating BIGNUM.");
					}

					if(BN_set_word(bignum.get(), RSA_F4) <= 0) {
						throw runtime_error("Error setting public exponent.");
					}

					auto rsa = RSA_PTR(RSA_new(),RSA_free);
					if (!bignum) {
						throw runtime_error("Error creating RSA.");
					}

 					if(!tpm2tss_rsa_genkey(rsa.get(), mbits, bignum.get(), NULL, 0)) {
						throw runtime_error("Error generating tpm2tss RSA key.");
					}

					TPM2_DATA tpm2Data;
					memcpy(&tpm2Data, RSA_get_app_data(rsa.get()), sizeof(tpm2Data));

					if (!tpm2tss_tpm2data_write(&tpm2Data, filename)) {
						throw runtime_error("Error writing TPM2 data to file.");
					}

					return ENGINE_load_private_key(engine,filename,NULL,NULL);

#else
					auto bignum = BIGNUM_PTR(BN_new(),BN_free);
					if (!bignum) {
						throw runtime_error("Error creating BIGNUM.");
					}

					if(BN_set_word(bignum.get(), RSA_F4) <= 0) {
						throw runtime_error("Error setting public exponent.");
					}

					auto ctx = CTX_PTR(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, engine),EVP_PKEY_CTX_free);
					if(!ctx) {
						throw SSL::Exception("EVP_PKEY_CTX_new_id");
					}

					if(EVP_PKEY_keygen_init(ctx.get()) <= 0) {
						throw SSL::Exception("EVP_PKEY_keygen_init");
					}
		
					if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), mbits) <= 0) {
						throw SSL::Exception("EVP_PKEY_CTX_set_rsa_keygen_bits");
					}

					if(EVP_PKEY_CTX_set_rsa_keygen_pubexp(ctx.get(), bignum.get()) <= 0) {
						throw runtime_error("Error setting RSA keygen public exponent.");
					}					

					EVP_PKEY *pkey;
					if(EVP_PKEY_keygen(ctx.get(), &pkey) <= 0) {
						throw SSL::Exception("EVP_PKEY_keygen");
					}

					return pkey;
#endif // HAVE_TPM2_TSS_ENGINE_H
						
				}

			};

			return make_shared<EngineBackEnd>();

#else
			throw runtime_error("Unable to use OpenSSL engine");
			break;
#endif // HAVE_OPENSSL_ENGINE

		case 2: // provider
#ifdef HAVE_OPENSSL_PROVIDER

			// Require package tpm2-openssl

			class ProviderBackEnd : public SSL::BackEnd {
			public:
				string name;
				OSSL_PROVIDER *provider;

				ProviderBackEnd() : name{Config::Value<string>{"ssl","provider-engine","tpm2"}.c_str()} {

					provider = OSSL_PROVIDER_load(NULL, name.c_str());
					if(!provider) {
						throw runtime_error(String{"Could not load OpenSSL provider '",name.c_str(),"'"});
					}
					Logger::String{"Using OpenSSL provider '",name.c_str(),"' for private key"}.trace();
				}

				~ProviderBackEnd() override {
					OSSL_PROVIDER_unload(provider);
				}

				EVP_PKEY * generate(const char *filename, const char *password, size_t mbits) override {
					EVP_PKEY *pkey = EVP_PKEY_Q_keygen(NULL, String{"provider=",name}.c_str(), "RSA", mbits);
					if(pkey && filename && *filename) {
						save(pkey, filename, password);
					}
					return pkey;
				}

			};

			return make_shared<ProviderBackEnd>();
#else
			throw runtime_error("Unable to use OpenSSL providers");
			break;
#endif // HAVE_OPENSSL_PROVIDER

		}

		throw SSL::Exception(String{"Unable to activate SSL backend '",name.c_str(),"'"});

	}

	SSL::Key::Private::Private(const char *filename, const char *password, bool autogenerate) {

		if(filename && *filename) {
			this->filename = filename;

			if(access(filename,R_OK) == 0) {
				// File exists, load it.
				load(filename,password);	
				return;
			}
		}

		if(autogenerate) {
			generate(filename, password, (size_t) Config::Value<unsigned int>{"ssl","mbits",2048}.get());
		}

		if(filename && *filename) {
			throw system_error(errno,system_category(),filename);
		}

		throw system_error(EINVAL,system_category(),"filename is required to load private key");

	}

	SSL::Key::Private::~Private() {

		if(pkey) {
			EVP_PKEY_free(pkey);
			pkey = NULL;
		}

	}

	void SSL::Key::Private::generate(const char *filename, const char *passwd, size_t mbits, const char *defmode) {

		if(pkey) {
			EVP_PKEY_free(pkey);
			pkey = NULL;
		}

		if(filename && *filename) {
			this->filename = filename;
		} else {
			this->filename.clear();
		}

		String mode;
		if(defmode && *defmode) {
			mode = defmode;
		} else {
			mode = Config::Value<string>{"ssl","genkey","auto"}.c_str();
		}

		backend = BackEndFactory(mode);
		pkey = backend->generate(filename, passwd, (size_t) Config::Value<unsigned int>{"ssl","mbits",2048});
		if(!pkey) {
			throw SSL::Exception("Error generating private key");
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

	std::string SSL::Key::Private::to_string() const {

		if(!pkey) {
			throw SSL::Exception("Private key is not loaded");
		}

		return backend->toString(pkey,filename.c_str());

	}

 }

