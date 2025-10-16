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
 #include <udjat/tools/memory.h>
 #include <cstdio>
 #include <memory>

 #ifdef HAVE_UNISTD_H
 #include <unistd.h>
 #endif // HAVE_UNISTD_H

 #define OPENSSL_SUPPRESS_DEPRECATED
 #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
 #include <openssl/engine.h>

 #ifdef HAVE_TPM2_TSS_ENGINE_H
 #include <tpm2-tss-engine.h>
 // using RSA_PTR = std::unique_ptr<RSA, decltype(&RSA_free)>;
 #endif // HAVE_TPM2_TSS_ENGINE_H

 using namespace std;

 namespace Udjat {

	class UDJAT_PRIVATE SSLEngine : public Crypto::BackEnd {
	public:
		ENGINE *engine;

		SSLEngine();
		~SSLEngine();
		void load(const char *filename, const char *password) override;
		void generate(const char *filename, const char *password, size_t mbits) override;

	};

	std::shared_ptr<Crypto::BackEnd> Crypto::BackEnd::EngineFactory() {
		return make_shared<SSLEngine>();
	}

	SSLEngine::SSLEngine() : Crypto::BackEnd{"engine","tpm2tss"} {
		ENGINE_load_builtin_engines();
		engine = ENGINE_by_id(type.c_str());
		if(!engine) {
			throw runtime_error(String{"Could not load OpenSSL engine '",type.c_str(),"'"});
		}

		if(ENGINE_init(engine) == 0) {
			throw runtime_error(String{"Could not initialize OpenSSL engine '",type.c_str(),"'"});
		}

		Logger::String{"Using OpenSSL engine '",type.c_str(),"' for private key"}.trace();
	}

	SSLEngine::~SSLEngine() {
		ENGINE_finish(engine);
	}

	void SSLEngine::load(const char *filename, const char *password) {
		if(!(filename && *filename)) {
			throw runtime_error("Filename is required to load key from engine.");
		}

		struct {
			const void *password;
			const char *prompt_info;
		} key_cb = { (void *) password, NULL };

		debug("Loading \n",filename);
		pkey = ENGINE_load_private_key(engine, filename, NULL, &key_cb);
	}

#if defined(HAVE_TPM2_TSS_ENGINE_H)
	void SSLEngine::generate(const char *filename, const char *password, size_t mbits) {

		// Reference: https://github.com/tpm2-software/tpm2-tss-engine/blob/master/src/tpm2tss-genkey.c
		auto bignum = make_handle(BN_new(),BN_free);
		if (!bignum) {
			throw runtime_error("Error creating BIGNUM.");
		}

		if(BN_set_word(bignum.get(), RSA_F4) <= 0) {
			throw runtime_error("Error setting public exponent.");
		}

		// auto rsa = RSA_PTR(RSA_new(),RSA_free);
		auto rsa = make_handle(RSA_new(),RSA_free);
		if (!rsa) {
			throw runtime_error("Error creating RSA.");
		}

		if(!tpm2tss_rsa_genkey(rsa.get(), mbits, bignum.get(), NULL, 0)) {
			throw runtime_error("Error generating tpm2tss RSA key.");
		}

		TPM2_DATA tpm2Data;
		memcpy(&tpm2Data, RSA_get_app_data(rsa.get()), sizeof(tpm2Data));

		pkey = tpm2tss_rsa_makekey(&tpm2Data);
		if (!pkey) {
			throw runtime_error("Error creating EVP_PKEY from TPM2 data.");
		}

		if (!tpm2tss_tpm2data_write(&tpm2Data, filename)) {
			throw runtime_error("Error writing TPM2 data to file.");
		}

	}
#else
	void SSLEngine::generate(const char *filename, const char *password, size_t mbits) {
		throw system_error(ENOTSUP,system_category(),"Internal engine genkey is not implemented");;
	}
#endif // HAVE_TPM2_TSS_ENGINE_H

 }

