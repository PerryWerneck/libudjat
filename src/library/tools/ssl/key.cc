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


 namespace Udjat {

	SSL::Key::Key(const char *filename, const char *password, bool autogenerate) {

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

	SSL::Key::~Key() {

		if(pkey) {
			EVP_PKEY_free(pkey);
			pkey = NULL;
		}

	}

	void SSL::Key::generate(const char *filename, const char *passwd, size_t mbits, const char *defmode) {

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

		backend = BackEnd::Factory(mode);
		pkey = backend->generate(filename, passwd, (size_t) Config::Value<unsigned int>{"ssl","mbits",2048});
		if(!pkey) {
			throw SSL::Exception("Error generating private key");
		}

	}

	void SSL::Key::load(const char *filename, const char *passwd) {

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

		backend = BackEnd::Factory(mode);
		pkey = backend->load(text,passwd);

	}

	void SSL::Key::save_private(const char *filename, const char *password) {
		if(!pkey) {
			throw SSL::Exception("Private key is not loaded");
		}
		backend->save_private(pkey, filename, password);
	}

	void SSL::Key::save_public(const char *filename) {
		if(!pkey) {
			throw SSL::Exception("Private key is not loaded");
		}
		backend->save_public(pkey, filename);
	}

	std::string SSL::Key::to_string() const {
		if(!pkey) {
			throw SSL::Exception("Private key is not loaded");
		}
		return backend->get_private(pkey,filename.c_str());
	}


 }

