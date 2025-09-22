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
 #include <udjat/tools/subprocess.h>
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

			debug("File ",filename," does not exist or is not readable");
		}

		if(autogenerate) {
			Logger::String{"Generating new private key in ",filename}.trace();
			generate(filename, password, (size_t) Config::Value<unsigned int>{"ssl","mbits",2048}.get());
			return;
		}

		throw system_error(EINVAL,system_category(),"filename is required to load private key");

	}

	SSL::Key::~Key() {

		if(pkey) {
			EVP_PKEY_free(pkey);
			pkey = NULL;
		}

	}

	bool SSL::Key::subproc(const char *filename, const char *passwd, size_t mbits, const char *type) {

		Config::Value<String> subproc{"ssl",String{type,"-genkey"}.c_str()};
		if(subproc.empty()) {
			Logger::String{"No subprocess defined to generate private key of type '",type,"'"}.write(Logger::Debug);
			return false;
		}

		string tempfilename = File::Temporary::create();
		subproc.expand([&](const char *key, std::string &str) {
			if(strcasecmp(key,"keyfile") == 0) {
				str = tempfilename;
			} else if(strcasecmp(key,"keysize") == 0) {
				str = std::to_string(mbits);
			} else if(strcasecmp(key,"password") == 0) {
				str = passwd ? passwd : "";
			}
			return true;
		}, true, true);

		Logger::String{"Running '",subproc.c_str(),"' to generate private key"}.trace();

		int rc = SubProcess{
			"keygen",
			subproc.c_str(),
			Logger::Info,
			Logger::Error
		}.run();

		if(rc) {
			throw runtime_error(String{"Error generating private key, subprocess returned ",rc});
		}

		File::Text text{tempfilename.c_str()};
		text.save(filename);

		unlink(tempfilename.c_str());

		return true;
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

		if(!this->subproc(filename, passwd, mbits, backend->type)) {

			// No subprocess defined, use the backend to generate the key.
			pkey = backend->generate(filename, passwd, mbits);
			if(!pkey) {
				throw SSL::Exception("Error generating private key");
			}

		} else {

			// Key was generated using subprocess, load it.
			debug("Loading '",filename,"'");
			pkey = backend->load(filename, passwd);
			if(!pkey) {
				throw SSL::Exception(String{"Error loading generated key from ",filename});
			}

		}

	}

	void SSL::Key::load(const char *filename, const char *passwd) {

        static const char *tpm2_tags[] = {
                "-----BEGIN TSS2 PRIVATE KEY-----",
                "-----BEGIN TSS2 KEY BLOB-----",
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
#if defined(HAVE_OPENSSL_ENGINE)
				mode = Config::Value<string>{"ssl","tpm-mode","engine"}.c_str();
#elif defined(HAVE_OPENSSL_PROVIDER)
				mode = Config::Value<string>{"ssl","tpm-mode","provider"}.c_str();
#else
				throw runtime_error("No OpenSSL backend available for TPM2 key");
#endif
				break;
			}
		}

		backend = BackEnd::Factory(mode);
		pkey = backend->load(filename,passwd);
		if(!pkey) {
			throw SSL::Exception(String{"Error loading private key from ",filename});
		}

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

