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
 #include <private/ssl.h>
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
 #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
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

 using RSA_PTR = std::unique_ptr<RSA, decltype(&RSA_free)>;

 namespace Udjat {

#if defined(HAVE_TPM2_TSS_ENGINE_H)

 static void tpm2_tss_genkey(const char *filename, const char *password, size_t mbits) {

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
 
}
#endif // HAVE_TPM2_TSS_ENGINE_H

	void SSL::Key::BackEnd::save_private(EVP_PKEY *pkey, const char *filename, const char *password) {

		FILE *file = fopen(filename, "w");
		if(!file) {
			throw system_error(errno,system_category(),filename);
		}

		if(PEM_write_PrivateKey(
			file, 
			pkey, 
			EVP_des_ede3_cbc(), 
			(unsigned char *) password, 
			(password ? strlen(password) : 0), 
			NULL, 
			NULL
		) != 1) {
			fclose(file);
			throw SSL::Exception("PEM_write_PrivateKey failed");
		}

		fclose(file);

	}

	void SSL::Key::BackEnd::save_public(EVP_PKEY *pkey, const char *filename) {

		FILE *file = fopen(filename, "w");
		if(!file) {
			throw system_error(errno,system_category(),filename);
		}

		if(PEM_write_PUBKEY(file,pkey) != 1) {
			fclose(file);
			throw SSL::Exception("PEM_write_PUBKEY failed");
		}

		fclose(file);

	}

	static int pcb(char *buf, int size, int rwflag, const char *password) {
		int len = strlen(password);
		if(len > size) {
			len = size;
		}
		memcpy(buf,password,len);
		return len;
	}

	EVP_PKEY * SSL::Key::BackEnd::load(const File::Text &file, const char *, const char *password) {

		debug("Loading \n",file.c_str());

		auto bio = BIO_PTR(BIO_new_mem_buf((void*)file.c_str(), -1),BIO_free_all);
		if(!bio) {
			throw SSL::Exception("BIO_new_mem_buf");
		}

		if(password && *password) {
			return PEM_read_bio_PrivateKey(bio.get(), NULL, (pem_password_cb *) pcb, (void *) password);
		}

		return PEM_read_bio_PrivateKey(bio.get(), NULL, NULL, NULL);
		
	}

	std::string SSL::Key::BackEnd::get_private(EVP_PKEY *pkey, const char *) {

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

	std::string SSL::Key::BackEnd::get_public(EVP_PKEY *pkey) {

		BIO_PTR bio(BIO_new(BIO_s_mem()), BIO_free_all);
		if(!bio) {
			throw SSL::Exception("BIO_new failed");
		}

		if(PEM_write_bio_PUBKEY(bio.get(), pkey) != 1) {
			throw SSL::Exception("PEM_write_bio_PUBKEY failed");
		}

		char *data;
		long len = BIO_get_mem_data(bio.get(), &data);
		return std::string(data, len);
	}
	
	std::shared_ptr<SSL::Key::BackEnd> SSL::Key::BackEnd::Factory(Udjat::String name) {

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

		/// @brief Legacy backend for OpenSSL.
		/// This backend uses the legacy OpenSSL API to generate and manage keys.
		class LegacyBackEnd : public BackEnd {
		public:
			LegacyBackEnd() : BackEnd{"legacy"} {
				Logger::String{"Using legacy OpenSSL backend for private key"}.trace();
			};

			EVP_PKEY * generate(const char *filename, const char *password, size_t mbits) override {
				EVP_PKEY *key = EVP_RSA_gen(mbits);
				if(key && filename && *filename) {
					save_private(key, filename, password);
				} else if(!key) {
					throw SSL::Exception("EVP_RSA_gen failed");
				}
				return key;
			}

		};

#ifdef HAVE_OPENSSL_PROVIDER

		/// @brief Provider backend for OpenSSL.
		/// This backend uses the OpenSSL provider API to generate and manage keys.
		/// It is used for TPM2 keys and other provider-based keys.
		/// Requires OpenSSL 3.0 or later.

		class ProviderBackEnd : public BackEnd {
		public:
			string name;
			OSSL_PROVIDER *provider;

			ProviderBackEnd() : BackEnd{"tpm2"}, name{Config::Value<string>{"ssl","provider-engine","tpm2"}.c_str()} {

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
				// https://github.com/tpm2-software/tpm2-openssl/blob/master/test/ec_genpkey_store_load.c
				// https://github.com/tpm2-software/tpm2-openssl/blob/master/test/rsa_genpkey_decrypt.c
				EVP_PKEY *pkey = EVP_PKEY_Q_keygen(NULL, String{"provider=",name.c_str()}.c_str(), "RSA", mbits);;
				if(!pkey) {
					Logger::String{"EVP_PKEY_Q_keygen failed for provider '",name.c_str(),"'"}.error();
				}
				if(pkey && filename && *filename) {
					save_private(pkey, filename, password);
				}
				return pkey;
			}

		};
#endif // HAVE_OPENSSL_PROVIDER

		switch(name.select("legacy","engine","provider","mixed",NULL)) {
		case 0: // Legacy
			return make_shared<LegacyBackEnd>();

		case 1: // engine
#ifdef HAVE_OPENSSL_ENGINE

			// Requires package openssl_tpm2_engine

			class EngineBackEnd : public BackEnd {
			public:
				ENGINE *engine;

				EngineBackEnd() : BackEnd{"tpm2"} {
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
				EVP_PKEY * load(const File::Text &, const char *filename, const char *password) override {

					if(!(filename && *filename)) {
						throw runtime_error("Filename is required to load key from TPM2TSS engine.");
					}

					struct {
                        const void *password;
                        const char *prompt_info;
                	} key_cb = { (void *) password, NULL };

					debug("Loading \n",filename);
					return ENGINE_load_private_key(engine, filename, NULL, &key_cb);

				}

				std::string get_private(EVP_PKEY *, const char *filename) override {
					if(!(filename && *filename)) {
						throw runtime_error("Filename is required to extract key from TPM2TSS engine.");
					}
					return File::Text{filename}.c_str();
				}
#endif // HAVE_TPM2_TSS_ENGINE_H

				EVP_PKEY * generate(const char *filename, const char *password, size_t mbits) override {

#if defined(HAVE_TPM2_TSS_ENGINE_H)

					tpm2_tss_genkey(filename, password, mbits);

					struct {
                        const void *password;
                        const char *prompt_info;
                	} key_cb = { (void *) password, NULL };

					debug("Loading \n",filename);

					return ENGINE_load_private_key(engine, filename, NULL, &key_cb);

#else
					Logger::String{"Internal engine genkey is not implemented, using subprocess"}.write(Logger::Debug,"sslkey");

					if(!subproc(filename, password, mbits, "tpm2tss")) {
						throw runtime_error("Unable to generate key using tpm2tss-genkey subprocess.");
					}

					struct {
                        const void *password;
                        const char *prompt_info;
                	} key_cb = { (void *) password, NULL };

					return ENGINE_load_private_key(engine, filename, NULL, &key_cb);

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
			return make_shared<ProviderBackEnd>();
#else
			throw runtime_error("Unable to use OpenSSL providers");
			break;
#endif // HAVE_OPENSSL_PROVIDER

		case 3: // mixed
			// Use tpm2-tss for key generation and provider for management
#ifdef HAVE_TPM2_TSS_ENGINE_H
			class MixedBackEnd : public ProviderBackEnd {
			public:
				MixedBackEnd() : ProviderBackEnd() {
				}

				EVP_PKEY * generate(const char *filename, const char *password, size_t mbits) override {

					Logger::String{"Using tpm2-tss instead of provider for private key generation"}.trace();
					tpm2_tss_genkey(filename, password, mbits);

					debug("Loading \n",filename);
					return this->load(File::Text{filename}, filename, password);
				}
			};
			return make_shared<MixedBackEnd>();
#else
			throw runtime_error("Unable to use mixed keys providers");
#endif

		}

		throw SSL::Exception(String{"Unable to activate SSL backend '",name.c_str(),"'"});

	}

 }

