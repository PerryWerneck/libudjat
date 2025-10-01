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
 #include <udjat/tools/subprocess.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #include <openssl/bio.h>
 #include <openssl/evp.h>
 #include <udjat/tools/crypto.h>
 #include <memory>

 #include <openssl/pem.h>
 #include <openssl/bio.h>

 using namespace std;

 // using RSA_PTR = std::unique_ptr<RSA, decltype(&RSA_free)>;

 namespace Udjat {

	Crypto::BackEnd::BackEnd(const char *name, const char *t) : type{Config::Value<string>{"crypto","type",t}.c_str()} {
	}

	Crypto::BackEnd::~BackEnd() {
		unload();
	}

	bool Crypto::BackEnd::subproc(const char *filename, const char *passwd, size_t mbits) {

		Config::Value<String> cmdline{"crypto",String{type.c_str(),"-genkey"}.c_str()};

		if(cmdline.empty()) {
			Logger::String{"No subprocess defined to generate private key for type '",type.c_str(),"'"}.trace();
			return false;
		}
		
		// Have subprocess to generate the key, use it.
		string tempfilename = File::Temporary::create();
		cmdline.expand([&](const char *key, std::string &str) {
			if(strcasecmp(key,"keyfile") == 0) {
				str = tempfilename;
			} else if(strcasecmp(key,"keysize") == 0) {
				str = std::to_string(mbits);
			} else if(strcasecmp(key,"password") == 0) {
				str = passwd ? passwd : "";
			}
			return true;
		}, true, true);

		debug("Running subprocess to generate private key: ",cmdline.c_str());

		int rc = SubProcess{
			"keygen",
			cmdline.c_str(),
			Logger::Info,
			Logger::Warning
		}.run();

		if(rc) {
			throw runtime_error(String{"Error generating private key, subprocess returned ",rc});
		}

		File::Text text{tempfilename.c_str()};
		text.save(filename);

		load(filename, passwd);

#ifndef DEBUG 
		unlink(tempfilename.c_str());
#endif // DEBUG

		return true;
	}

	void Crypto::BackEnd::save_private(const char *filename, const char *password) {

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
			throw Crypto::Exception("PEM_write_PrivateKey failed");
		}

		fclose(file);

	}

	void Crypto::BackEnd::save_public(const char *filename) {

		FILE *file = fopen(filename, "w");
		if(!file) {
			throw system_error(errno,system_category(),filename);
		}

		if(PEM_write_PUBKEY(file,pkey) != 1) {
			fclose(file);
			throw Crypto::Exception("PEM_write_PUBKEY failed");
		}

		fclose(file);

	}

	std::string Crypto::BackEnd::get_private() {

		BIO_PTR bio(BIO_new(BIO_s_mem()), BIO_free_all);
		if(!bio) {
			throw Crypto::Exception("BIO_new failed");
		}

		if(PEM_write_bio_PrivateKey(bio.get(), pkey, NULL, NULL, 0, NULL, NULL) != 1) {
			throw Crypto::Exception("PEM_write_bio_PrivateKey failed");
		}

		char *data;
		long len = BIO_get_mem_data(bio.get(), &data);
		return std::string(data, len);
	}

	std::string Crypto::BackEnd::get_public() {

		BIO_PTR bio(BIO_new(BIO_s_mem()), BIO_free_all);
		if(!bio) {
			throw Crypto::Exception("BIO_new failed");
		}

		if(PEM_write_bio_PUBKEY(bio.get(), pkey) != 1) {
			throw Crypto::Exception("PEM_write_bio_PUBKEY failed");
		}

		char *data;
		long len = BIO_get_mem_data(bio.get(), &data);
		return std::string(data, len);
	}

	std::shared_ptr<Crypto::BackEnd> Crypto::BackEnd::Factory(Udjat::String name) {

		if(strcasecmp(name.c_str(),"auto") == 0) {

			// Check if tpm is available
			if(access("/dev/tpm0",F_OK) != 0) {
				name = "legacy";
			} else {
#if defined(HAVE_OPENSSL_ENGINE)
				// /usr/lib64/engines-3/tpm2.so
				name = Config::Value<string>{"crypto","tpm2-backend","engine"}.c_str();
#elif defined(HAVE_OPENSSL_PROVIDER)
				// /usr/lib64/ossl-modules/tpm2.so
				name = Config::Value<string>{"crypto","tpm2-backend","provider"}.c_str();
#else
				name = Config::Value<string>{"crypto","tpm2-backend","legacy"}.c_str();
#endif
			}
		}

		switch(name.select("legacy","engine","provider",NULL)) {
		case 0: // Legacy
			return LegacyFactory();

		case 1: // engine
#ifdef HAVE_OPENSSL_ENGINE
			return EngineFactory();
#else
			throw runtime_error("Unable to use OpenSSL engine");
#endif // HAVE_OPENSSL_ENGINE

		case 2: // provider
#ifdef HAVE_OPENSSL_PROVIDER
			// Require package tpm2-openssl
			return ProviderFactory();
#else
			throw runtime_error("Unable to use OpenSSL providers");
#endif // HAVE_OPENSSL_PROVIDER	

		default:
			throw Crypto::Exception(String{"Unable to activate crypto backend '",name.c_str(),"'"});

		}

	}

 }

